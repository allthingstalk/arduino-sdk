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

// Define http and mqtt endpoints
#define httpServer "api.allthingstalk.io"  // API endpoint
#define mqttServer "api.allthingstalk.io"  // broker

ATTDevice device;

int LIGHT = 0;  // analog 0 is the input pin, this corresponds with the number on the Grove shield where the Lightsensor is attached to

void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

void setup()
{
  pinMode(LIGHT, OUTPUT);
  
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
    
  device.addAsset("Light", "Lightsensor", "light sensor", "sensor", "integer");  // Create the asset for your device
  
  while(!device.subscribe(pubSub))  // make certain that we can receive message from the iot platform (activate mqtt)
    Serial.println("retrying");     
}

unsigned long sysTime;  // only send every x milliseconds
void loop()
{
  unsigned long curTime = millis();
  if (curTime > (sysTime + 5000))  // enough time has passed
  {
    unsigned int value = analogRead(LIGHT);  // read from light sensor
    device.send(String(value), "Light");
    sysTime = curTime;
  }
  Device.Process(); 
}


// callback function: handles messages that were sent from the iot platform to this device.
void callback(char* topic, byte* payload, unsigned int length) 
{
}