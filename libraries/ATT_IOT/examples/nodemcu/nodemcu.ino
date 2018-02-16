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

//#define JSON
#define CBOR

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Define http and mqtt endpoints
#define http "api.allthingstalk.io"  // API endpoint
#define mqtt "api.allthingstalk.io"  // broker

#include <ATT_IOT.h>
#include <SPI.h>  // required to have support for signed/unsigned long type.

void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espClient;
PubSubClient pubSub(mqtt, 1883, callback, espClient);

ATTDevice device;

#ifdef CBOR
  #include <CborBuilder.h>
  CborBuilder payload(device);
#endif

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the onboard LED off
  
  Serial.begin(9600);  // Init serial link for debugging
  
  // Enter your WiFi credentials here!
  setupWiFi("", "");
  //
  
  while(!device.connect(&espClient, http))  // Connect to AllThingsTalk
    Serial.println("retrying");

  // Create device assets
  //device.addAsset("counter", "counter", "counting up", "sensor", "{\"type\": \"integer\"}");
  //device.addAsset("toggle", "toggle", "toggle", "actuator", "{\"type\": \"boolean\"}");
  
  while(!device.subscribe(pubSub))  // Subscribe to mqtt
    Serial.println("retrying"); 
}

void setupWiFi(const char* ssid, const char* password)
{
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
}

unsigned long prevTime;
unsigned int prevVal = 0;
int counter = 0;
void loop()
{
  unsigned long curTime = millis();
  if (curTime > (prevTime + 5000))  // Update and send counter value every 5 seconds
  {
    #ifdef JSON
    device.send(String(counter), "counter");
    #endif

    #ifdef CBOR  // Send data using Cbor
    payload.reset();
    payload.map(1);
    payload.addInteger(counter, "counter");
    payload.send();
    #endif
    
    counter++;
    prevTime = curTime;
  }
  device.process();  // Check for incoming messages
}

/****
 * Callback function
 * Handle messages that were sent from the AllThingsTalk cloud to this device
 */
void callback(char* topic, byte* payload, unsigned int length) 
{ 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert payload to json
  StaticJsonBuffer<500> jsonBuffer;
  char json[500];
  for (int i = 0; i < length; i++) {
    json[i] = (char)payload[i];
  }
  json[length] = '\0';
  
  JsonObject& root = jsonBuffer.parseObject(json);

  // Do something
  if(root.success())
  {
    const char* value = root["value"];
    Serial.println(value);
    if (strcmp(value,"true") == 0)
      digitalWrite(LED_BUILTIN, LOW);   // Turn the onboard LED on
    else
      digitalWrite(LED_BUILTIN, HIGH);  // Turn the onboard LED off

    device.send(value, "toggle");  // Send command back as ACK using JSON
  }
  else
    Serial.println("Parsing JSON failed");
}