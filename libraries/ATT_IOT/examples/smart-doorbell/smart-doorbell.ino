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

//#include <Ethernet2.h>
#include <Ethernet.h>
#include <EthernetClient.h>

#include <PubSubClient.h>

#include <ATT_IOT.h>  // AllThingsTalk for Makers Arduino Library
#include <SPI.h>      // required to have support for signed/unsigned long type                   

// define http and mqtt endpoints
#define httpServer "api.allthingstalk.io"  // API endpoint
#define mqttServer "api.allthingstalk.io"  // broker

ATTDevice device;

int DOORBELL = 8;

void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

void setup()
{
  pinMode(DOORBELL, INPUT);
  
  Serial.begin(9600);  // init serial link for debugging
  
  byte mac[] = {  0x90, 0xA2, 0xDA, 0x0D, 0x8D, 0x3D };  // adapt to your Arduino MAC Address  
  if (Ethernet.begin(mac) == 0)                          // initialize the Ethernet connection:
  { 
    Serial.println(F("DHCP failed,end"));
    while(true);  // we failed to connect, halt execution here. 
  }
  delay(1000);
  
  while(!device.connect(&ethClient, httpServer))  // connect the device with the IOT platform.
    Serial.println("retrying");
    
  device.addAsset("Doorbell", "Doorbell", "Doorbell button", "sensor", "boolean");  // create the asset for your device
  
  while(!device.subscribe(pubSub))  // make certain that we can receive message from the iot platform (activate mqtt)
    Serial.println("retrying");     
}

bool button = false;
void loop()
{
  bool buttonValue = digitalRead(DOORBELL);  // read button status
  if (button != buttonValue)                 // verify if value has changed
  {
    button = buttonValue;
    delay(100);
    if (buttonValue == 1)
      device.send("true", "Doorbell");
    else
      device.send("false", "Doorbell");
  }
  device.process(); 
}

// callback function: handles messages that were sent from the iot platform to this device.
void callback(char* topic, byte* payload, unsigned int length) 
{
}