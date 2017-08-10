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
#include <SPI.h>  // required to have support for signed/unsigned long type

// define device credentials
char deviceId[] = "awhwKcb1KdKX9ILDCQrX4rFf";
char token[] = "maker:4LIZEoplVnzXm0lqFv4veKAwOWC1qvpQOFtTeVQ";

// define http and mqtt endpoints
#define httpServer "api.allthingstalk.io"  // API endpoint
#define mqttServer "api.allthingstalk.io"  // broker

ATTDevice device(deviceId, token);

int ledPin = 2;  // our LED is connected to pin 2 on the Arduino

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
    while(true);  // we failed to connect, halt execution here
  }
  delay(1000);  // give the Ethernet shield a second to initialize
  
  while(!device.connect(&ethClient, httpServer))  // connect the device with AllThingsTalk
    Serial.println("retrying");

  device.addAsset("led", "led", "light emitting diode", "actuator", "boolean");
  while(!device.subscribe(pubSub))  // make certain that we can receive messages over mqtt
    Serial.println("retrying"); 
}

void loop()
{
  device.process();  // check callback for incoming messages
}


// callback function
// handle messages that were sent from the AllThingsTalk cloud to this device
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String msgString; 
  String assetName = device.getAssetName(topic, strlen(topic));
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

  {
    if(assetName == "led")
    {
      if (msgString.indexOf("false") > -1) {
        digitalWrite(ledPin, LOW);
        device.send("false", "led");
      }
      else if (msgString.indexOf("true") > -1) {
        digitalWrite(ledPin, HIGH);
        device.send("true", "led");
      }
    }
  }
}
