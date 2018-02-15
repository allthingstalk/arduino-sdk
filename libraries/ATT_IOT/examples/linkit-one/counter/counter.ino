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

#include <LGPS.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LGPRSUdp.h>
#include <LBattery.h>

#include <Wire.h>

#include <PubSubClient.h>
#include <ATT_IOT.h>

// Define http and mqtt endpoints
#define http "api.allthingstalk.io"  // API endpoint
#define mqtt "api.allthingstalk.io"  // broker

void callback(char* topic, byte* payload, unsigned int length);
void connectToPlatform();
unsigned long getTime();

float calc_dist(float flat1, float flon1, float flat2, float flon2);

LGPRSClient c;
PubSubClient pubSub(mqtt, 1883, callback, c);

ATTDevice device;

void setup()
{
  Serial.begin(115200);
  while(!Serial);  // Wait until the serial monitor is initialized correctly

  Serial.println("Starting");
  
  while(!LGPRS.attachGPRS("internet.be", "",""))
  {
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  
  while(!device.connect(&c, http))  // Connect to AllThingsTalk
    Serial.println("retrying");
  
  while(!device.subscribe(pubSub))  // Subscribe to mqtt
    Serial.println("retrying"); 
}

unsigned long prevTime;
unsigned int prevVal = 0;
int counter = 0;
void loop()
{
  unsigned long curTime = millis();
  if (curTime > (prevTime + 5000))  // Update and send counter value every 5 seconds
  {
    device.send(String(counter), "counter");
    
    counter++;
    prevTime = curTime;
  }
  device.process();  // Check for incoming messages
}

void callback(char* topic, byte* payload, unsigned int length) 
{ 
}