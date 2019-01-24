/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2017 AllThingsTalk
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define DEBUG 1 // Toggle debugging in the IOT library
#define FAST_MQTT


#include "ATT_IOT.h"

#define RETRYDELAY 5000     // the nr of milliseconds that we pause before retrying to create the connection
#define ETHERNETDELAY 1000  // the nr of milliseconds that we pause to give the ethernet board time to start
#define MQTTPORT 1883

#if DEBUG
char HTTPSERVTEXT[] = "connection HTTP Server";
char MQTTSERVTEXT[] = "connection MQTT Server";
char FAILED_RETRY[] = " failed,retry";
char SUCCESTXT[] = " established";
#endif

//create the object
//ATTDevice::ATTDevice(): _client(NULL), _mqttclient(NULL)
//{
//  _deviceId = DEVICE_ID;
//  _token = DEVICE_TOKEN;
//}

//create the object
ATTDevice::ATTDevice(String deviceId, String deviceToken): _client(NULL), _mqttclient(NULL)
{
  _deviceId = deviceId;
  _token = deviceToken;
}

void ATTDevice::setCredentials(String deviceId, String token)
{
  _deviceId = deviceId;
  _token = token;
}

// connect with the http server
bool ATTDevice::connect(Client* httpClient, const char httpServer[])
{
  _client = httpClient;
  _serverName = httpServer;  // keep track of this value while working with the http server

  #if DEBUG
  Serial.print("Connecting to ");
  Serial.println(httpServer);
  #endif

  if (!_client->connect(httpServer, 80))  // if you get a connection, report back via serial:
  {
    #if DEBUG
    Serial.print(HTTPSERVTEXT);
    Serial.println(FAILED_RETRY);
    #endif
    return false;  // we have created a connection succesfully
  }
  else{
    #if DEBUG
    Serial.print(HTTPSERVTEXT);
    Serial.println(SUCCESTXT);
    #endif
    delay(ETHERNETDELAY);  // another small delay: sometimes the card is not yet ready to send the asset info.
    return true;           // we have created a connection succesfully
  }
}

// closes any open connections (http & mqtt) and resets the device. After this call,
// you can call connect and/or subscribe again. Credentials remain stored
void ATTDevice::close()
{
  closeHTTP();
  closeMQTT();
}

// closes the http connection, if any
void ATTDevice::closeHTTP()
{
  if(_client){
    #if DEBUG
    Serial.println(F("Stopping HTTP"));
    #endif
    _client->flush();
    _client->stop();
    _client = NULL;
  }
}

void ATTDevice::closeMQTT()
{
  _mqttUserName = NULL;
  _mqttpwd = NULL;
  if(_mqttclient){
    _mqttclient->disconnect();
    _mqttclient = NULL;
  }
}

// create or update the specified asset.
void ATTDevice::addAsset(String name, String title, String description, String assetType, String dataType)
{
  // Make a HTTP request:
  {
    _client->println("PUT /device/" + _deviceId + "/asset/" + name  + " HTTP/1.1");
  }
    _client->print(F("Host: "));
    _client->println(_serverName);
    _client->println(F("Content-Type: application/json"));
    _client->print(F("Authorization: Bearer "));_client->println(_token);

  _client->print(F("Content-Length: "));
  {                                          //make every mem op local, so it is unloaded asap
    int length = title.length() + description.length() + dataType.length();
    if(assetType.equals("sensor"))
      length += 6;
    else if(assetType.equals("actuator"))
      length += 8;
     else if(assetType.equals("virtual"))
      length += 7;
     else if(assetType.equals("config"))
      length += 6;

    if (dataType.length() == 0)
      length += 39;
    else if(dataType[0] == '{')
      length += 49;
    else
      length += 62;
    _client->println(length);
  }
    _client->println();

  _client->print(F("{\"title\":\""));
  _client->print(title);
  _client->print(F("\",\"description\":\""));
  _client->print(description);
  _client->print(F("\",\"is\":\""));
  _client->print(assetType);
  if(dataType.length() == 0)
    _client->print(F("\""));
  else if(dataType[0] == '{'){
    _client->print(F("\",\"profile\": "));
    _client->print(dataType);
  }
  else{
    _client->print(F("\",\"profile\": { \"type\":\""));
    _client->print(dataType);
    _client->print(F("\" }"));
  }
  _client->print(F("}"));
  _client->println();
  _client->println();

  unsigned long maxTime = millis() + 1000;
  while(millis() < maxTime)    //wait, but for the minimum amount of time.
  {
    if(_client->available()) break;
    else delay(10);
  }
  getHTTPResult();      //get the response from the server and show it.
}

//connect with the http server and broker
bool ATTDevice::subscribe(PubSubClient& mqttclient)
{
  Serial.println("subscribing");
  if(_token){
    return subscribe(mqttclient, _token.c_str());
  }
  else{
    #if DEBUG
    Serial.print(MQTTSERVTEXT);
    Serial.println("failed: invalid credentials");
    #endif
    return false;
  }
}

// Stop http processing & make certain that we can receive data from the mqtt server, given the specified username and pwd.
// This Subscribe function can be used to connect to a fog gateway
// returns true when successful, false otherwise
bool ATTDevice::subscribe(PubSubClient& mqttclient, const char* username)
{
  _mqttclient = &mqttclient;
  _serverName = "";  // no longer need this reference
  closeHTTP();  // close Http connection before opening Mqtt connection
  _mqttUserName = username;
  _mqttpwd = "";
  return mqttConnect();
}

// tries to create a connection with the mqtt broker. also used to try and reconnect.
bool ATTDevice::mqttConnect()
{
  char mqttId[23];  // or something long enough to hold the longest file name you will ever use
  int length = _deviceId.length();
  length = length > 22 ? 22 : length;
    _deviceId.toCharArray(mqttId, length);
  mqttId[length] = 0;
  if(_mqttUserName && _mqttpwd){
    if (!_mqttclient->connect(mqttId, (char*)_mqttUserName, (char*)_mqttpwd))
    {
      #if DEBUG
      Serial.print(MQTTSERVTEXT);
      Serial.println(FAILED_RETRY);
      #endif
      return false;
    }
    #if DEBUG
    Serial.print(MQTTSERVTEXT);
    Serial.println(SUCCESTXT);
    #endif
    mqttSubscribe();
    return true;
  }
  else{
    #if DEBUG
    Serial.print(MQTTSERVTEXT);
    Serial.println("failed: invalid credentials");
    #endif
    return false;
  }
}

// check for any new mqtt messages
bool ATTDevice::process()
{
  if(_mqttclient->connected() == false)
  {
    #if DEBUG
    Serial.println(F("Lost broker connection,restarting from process"));
    #endif
    if(mqttConnect() == false)
      return false;
  }
  _mqttclient->loop();
  return true;
}

// build the content that has to be sent to the cloud using mqtt (either a csv value or a json string)
char* ATTDevice::buildJsonContent(String value)
{
  char* message_buff;
  int length;
  if(value[0] == '[' || value[0] == '{'){
    length = value.length() + 16;
    message_buff = new char[length];
    sprintf(message_buff, "{\"value\":%s}", value.c_str());
  }
  else{
    length = value.length() + 3;
    message_buff = new char[length];
    sprintf(message_buff, "0|%s", value.c_str());
  }
  message_buff[length-1] = 0;
  return message_buff;
}


// send a data value to the cloud server for the sensor with the specified id
void ATTDevice::send(String value, String asset)
{
  if(_mqttclient->connected() == false)
  {
    #if DEBUG
    Serial.println(F("Lost broker connection,restarting from send"));
    #endif
    mqttConnect();
  }

  char* message_buff = buildJsonContent(value);

  #if DEBUG  // don't need to write all of this if not debugging
  Serial.print(F("Publish to ")); Serial.print(asset); Serial.print(": "); Serial.println(message_buff);
  #endif

  char* Mqttstring_buff;
  {
    int length = _deviceId.length() + 21 + asset.length();  // 21 fixed chars + deviceId + assetName
    Mqttstring_buff = new char[length];
    sprintf(Mqttstring_buff, "device/%s/asset/%s/state", _deviceId.c_str(), asset.c_str());
    Mqttstring_buff[length-1] = 0;
  }
  _mqttclient->publish(Mqttstring_buff, message_buff);
  #ifndef FAST_MQTT  // some boards like the old arduino ethernet need a little time after sending mqtt data, other boards don't.
  delay(100);        // give some time to the ethernet shield so it can process everything.
  #endif
  delete(message_buff);
  delete(Mqttstring_buff);
}

bool ATTDevice::sendBinary(void* packet, unsigned char size)
{
    if(_mqttclient->connected() == false)
  {
    #if DEBUG
    Serial.println(F("Lost broker connection,restarting from send"));
    #endif
    mqttConnect();
  }

  #if DEBUG  // don't need to write all of this if not debugging
  Serial.print(F("Publish to "));
  // Print actual payload from binary buffer
  char hexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	for (unsigned char i = 0; i < size; i++)
  {
		Serial.print(hexTable[((unsigned char*)packet)[i] / 16]);
 		Serial.print(hexTable[((unsigned char*)packet)[i] % 16]);
	}
  Serial.println();
  #endif

  char* Mqttstring_buff;
  {
    int length = _deviceId.length() + 14;  // 14 fixed chars + deviceId
    Mqttstring_buff = new char[length];
    sprintf(Mqttstring_buff, "device/%s/state", _deviceId.c_str());
    Mqttstring_buff[length-1] = 0;
  }
  _mqttclient->publish(Mqttstring_buff, (unsigned char*)packet, size);
  #ifndef FAST_MQTT  // some boards like the old arduino ethernet need a little time after sending mqtt data, other boards don't.
  delay(100);        // give some time to the ethernet shield so it can process everything.
  #endif
  delete(Mqttstring_buff);

  return true;
}

bool ATTDevice::sendCbor(unsigned char* data, unsigned int size)
{
  if(_mqttclient->connected() == false)
  {
    #if DEBUG
    Serial.println(F("Lost broker connection,restarting from send"));
    #endif
    mqttConnect();
  }

  #if DEBUG  // don't need to write all of this if not debugging
  Serial.print(F("Publish to "));
  // Print actual payload from binary buffer
  char hexTable[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	for (unsigned char i = 0; i < size; i++)
  {
		Serial.print(hexTable[data[i] / 16]);
 		Serial.print(hexTable[data[i] % 16]);
	}
  Serial.println();
  #endif

  char* Mqttstring_buff;
  {
    int length = _deviceId.length() + 14;  // 14 fixed chars + deviceId
    Mqttstring_buff = new char[length];
    sprintf(Mqttstring_buff, "device/%s/state", _deviceId.c_str());
    Mqttstring_buff[length-1] = 0;
  }
  _mqttclient->publish(Mqttstring_buff, data, size);
  #ifndef FAST_MQTT  // some boards like the old arduino ethernet need a little time after sending mqtt data, other boards don't.
  delay(100);        // give some time to the ethernet shield so it can process everything.
  #endif
  delete(Mqttstring_buff);

  return true;
}



// subscribe to the mqtt topic so we can receive data from the server
void ATTDevice::mqttSubscribe()
{
  String MqttString = "device/" + _deviceId + "/asset/+/command";
  char Mqttstring_buff[MqttString.length()+1];
    MqttString.toCharArray(Mqttstring_buff, MqttString.length()+1);
    _mqttclient->subscribe(Mqttstring_buff);

  #if DEBUG
    Serial.println("MQTT Client subscribed");
  #endif
}

// returns the pin nr found in the topic
String ATTDevice::getAssetName(char* topic, int topicLength)
{
  int i=0;
  char* command = strtok(topic, "/");
  while (command != 0)
  {
    if(i==3)  // 3rd section of topic contains asset name "device/<deviceId>/asset/<assetName>/command"
    {
      #if DEBUG
        Serial.println(command);
      #endif
      return command;
    }
    command = strtok(0, "/");  // next string section
    i++;
  }
  return "";
}

void ATTDevice::getHTTPResult()
{
  // if there's incoming data from the net connection, send it out the serial port
  // this is for debugging purposes only
  if(_client->available()){
    while (_client->available()) {
      char c = _client->read();
      #if DEBUG
      Serial.print(c);
      #endif
    }
    #if DEBUG
    Serial.println();
    #endif
  }
}
