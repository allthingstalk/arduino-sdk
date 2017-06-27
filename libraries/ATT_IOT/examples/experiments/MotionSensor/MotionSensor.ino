/*
   Copyright 2014-2017 AllThingsTalk

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

//#include <Ethernet2.h>
#include <Ethernet.h>
#include <EthernetClient.h>

#include <PubSubClient.h>

#include <ATT_IOT.h>  // AllThingsTalk for Makers Arduino Library
#include <SPI.h>      // required to have support for signed/unsigned long type        

/*
  PIR Motion Sensor AllThingsTalk Makers Arduino Example
  version 1.0 09/10/2014
  
  ### Instructions

  1. Setup the Arduino hardware
    - USB2Serial
    - Grove kit shield
    - Grove PIR Motion sensor to D2 and grove LEd to D8
  2. Add 'iot_att' library to your Arduino Environment. [Try this guide](http://arduino.cc/en/Guide/Libraries)
  3. fill in the missing strings (deviceId, clientId, clientKey, mac) and optionally change/add the sensor & actuator names, ids, descriptions, types
     For extra actuators, make certain to extend the callback code at the end of the sketch.
  4. Upload the sketch

  ### Troubleshooting


*/

// define device credentials and endpoint
char deviceId[] = "";
char token[] = "";
#define httpServer "tasty.allthingstalk.io"  // API endpoint

ATTDevice Device(deviceId, token);  // create the object that provides the connection to the cloud to manager the device
#define mqttServer httpServer       // MQTT Server Address

int PIR = 2;  // define PIN number on shield & also used to construct Unique AssetID                      
int LED = 8;  // define PIN number on shield & also used to construct Unique AssetID

//required for the device
void callback(char* topic, byte* payload, unsigned int length);
EthernetClient ethClient;
PubSubClient pubSub(mqttServer, 1883, callback, ethClient);

void setup()
{
  pinMode(PIR, INPUT);
  pinMode(LED, OUTPUT);
  
  Serial.begin(9600);  // init serial link for debugging
  
  byte mac[] = {  0x90, 0xA2, 0xDA, 0x0D, 0x8D, 0x3D };  // adapt to your Arduino MAC Address  
  if (Ethernet.begin(mac) == 0)                          // initialize the Ethernet connection:
  { 
    Serial.println(F("DHCP failed,end"));
    while(true);  // we failed to connect, halt execution here. 
  }
  delay(1000);
  
  while(!Device.Connect(&ethClient, httpServer))  // connect the device with the IOT platform.
    Serial.println("retrying");
    
  Device.AddAsset("PIR", "PIR", "Motion Sensor", "sensor", "boolean");           // create the asset for your device
  Device.AddAsset("LED", "LED", "Light Emitting Diode", "actuator", "boolean");  // create the asset for your device
  
  while(!Device.Subscribe(pubSub))  // make certain that we can receive message from the iot platform (activate mqtt)
    Serial.println("retrying");     
}

bool Motion = false;                                    
void loop()
{
  bool MotionRead = digitalRead(PIR);  // read status 
  if (Motion != MotionRead)  // verify if value has changed
  {
    Motion = MotionRead;
    delay(100);
    if (MotionRead == 1)
       Device.Send("true", "PIR");
    else
       Device.Send("false", "PIR");
  }
  Device.Process(); 
}


// callback function: handles messages that were sent from the iot platform to this device.
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  String msgString;
  String assetName;
  {
    char message_buff[length + 1];
    strncpy(message_buff, (char*)payload, length);
    message_buff[length] = '\0';
          
    msgString = String(message_buff);
  }

  {
    Serial.print("Payload: ");
    Serial.println(msgString);
    Serial.print("topic: ");
    Serial.println(topic);

    String assetName = Device.GetAssetName(topic, strlen(topic));
    
    if(assetName.equals("LED"))
    {
      msgString.toLowerCase();  // make sure we have 'true' and 'false' in lowercase for our boolean check
      if (msgString.indexOf("true") > -1) {
        digitalWrite(LED, HIGH);
        Device.Send("true", "LED");
      }
      else if (msgString.indexOf("false") > -1) {
        digitalWrite(LED, LOW);
        Device.Send("false", "LED");
      }
    }
  }
}