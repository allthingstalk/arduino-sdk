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

#include <Ethernet.h>
//#include <Ethernet2.h>
#include <EthernetClient.h>

#include <PubSubClient.h>

#include <ATT_IOT.h>
#include <SPI.h>  // required to have support for signed/unsigned long type.

// define device credentials and endpoint
char deviceId[] = "";
char token[] = "";
#define httpServer "api.allthingstalk.io"  // API endpoint

ATTDevice Device(deviceId, token);  // create the object that provides the connection to the cloud to manager the device.

#define mqttServer httpServer  // MQTT Server Address 

int ledPin = 8;   // our LED is connected to pin D8 on the Arduino
int knobPin = 0; // rotary sensor is connected to pin A0 on the Arduino

// required for the device
void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

void setup()
{
  pinMode(ledPin, OUTPUT);  // initialize the digital pin as an output
  Serial.begin(9600);       // init serial link for debugging
  
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0xE1, 0x3E};  // adapt to your Arduino MAC Address  
  if (Ethernet.begin(mac) == 0)                       // initialize the Ethernet connection
  { 
    Serial.println(F("DHCP failed,end"));
    while(true);                                      // we failed to connect, halt execution here
  }
  delay(1000);  // give the Ethernet shield a second to initialize
  
  while(!Device.Connect(&ethClient, httpServer))  // connect the device with the IOT platform
    Serial.println("retrying");
    
  Device.AddAsset("knob", "knob", "rotary switch", "sensor", "{\"type\": \"integer\", \"minimum\": 0, \"maximum\": 1023}");
  Device.AddAsset("led", "led", "light emitting diode", "actuator", "boolean");
  while(!Device.Subscribe(pubSub))  // make certain that we can receive message from the iot platform (activate mqtt)
    Serial.println("retrying"); 
}

unsigned long time;  // only send every x amount of time
unsigned int prevVal =0;
void loop()
{
  unsigned long curTime = millis();
  if (curTime > (time + 1000))
  {
    unsigned int knobRead = analogRead(knobPin);
    if(prevVal != knobRead){
      Device.Send(String(knobRead), "knob");
      prevVal = knobRead;
    }
    time = curTime;
  }
  Device.Process(); 
}


// Callback function: handles messages that were sent from the iot platform to this device.
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String msgString; 
  String assetName = Device.GetAssetName(topic, strlen(topic));
  {
    char message_buff[length + 1];
    strncpy(message_buff, (char*)payload, length);
    message_buff[length] = '\0';
          
    msgString = String(message_buff);
    msgString.toLowerCase();

    Serial.print("Payload: ");
    Serial.println(msgString);
    Serial.print("Topic  : ");
    Serial.println(topic);
  }

  // put this in a sub block, so any unused memory can be freed as soon as possible, required to save mem while sending data
  {
    if(assetName == "led")
    {
      if (msgString.indexOf("false") > -1) {
        digitalWrite(ledPin, LOW);
        Device.Send("false", "led");
      }
      else if (msgString.indexOf("true") > -1) {
        digitalWrite(ledPin, HIGH);
        Device.Send("true", "led");
      }
    }
  }
}