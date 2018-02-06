/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2018 AllThingsTalk
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


// Select your preferred method of sending data
#define JSON
//#define CBOR
//#define BINARY

/****************************************************************/
#include <Ethernet.h>
//#include <Ethernet2.h>

#include <EthernetClient.h>
#include <PubSubClient.h>
#include <keys.h>

#include <ATT_IOT.h>
#include <SPI.h>  // required to have support for signed/unsigned long type.

//required for the device
void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(MQTT_SERVER, 1883, callback, ethClient);

ATTDevice device(DEVICE_ID, DEVICE_TOKEN);

#ifdef CBOR
  #include <CborBuilder.h>
  CborBuilder payload(device);
#endif

#ifdef BINARY
  #include <PayloadBuilder.h>
  PayloadBuilder payload(device);
#endif

void setup()
{
  Serial.begin(9600);       // init serial link for debugging
  
  byte mac[] = {0x90, 0xA2, 0xDA, 0x0D, 0xE1, 0x3E};  // adapt to your Arduino MAC Address  
  if (Ethernet.begin(mac) == 0)                       // initialize the Ethernet connection
  { 
    Serial.println(F("DHCP failed,end"));
    while(true);  // we failed to connect, halt execution here
  }
  delay(1000);  // give the Ethernet shield a second to initialize
  
  while(!device.connect(&ethClient, HTTP_SERVER))  // connect the device with AllThingsTalk
    Serial.println("retrying");
    
  //device.addAsset("counter", "counter", "counting up", "sensor", "{\"type\": \"integer\"}");
  
  while(!device.subscribe(pubSub))  // make certain that we can receive messages over mqtt
    Serial.println("retrying"); 
}

unsigned long time;
unsigned int prevVal = 0;
int counter = 0;
void loop()
{
  unsigned long curTime = millis();
  if (curTime > (time + 5000))  // Update and send counter value every 5 seconds
  {
    #ifdef JSON
    device.send(String(counter), "counter");
    #endif

    #ifdef CBOR
    payload.reset();
    payload.map(1);
    payload.addInteger(counter, "counter");
    payload.send();
    #endif

    #ifdef BINARY
    payload.reset();
    payload.addInteger(counter);
    payload.send();
    #endif
    
    counter++;
    time = curTime;
  }
  device.process();  // Check for incoming messages
}

/****
 * Callback function
 * Handle messages that were sent from the AllThingsTalk cloud to this device
 */
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String assetName = device.getAssetName(topic, strlen(topic));
  Serial.print("Data arrived from asset: ");
  Serial.println(assetName);
}