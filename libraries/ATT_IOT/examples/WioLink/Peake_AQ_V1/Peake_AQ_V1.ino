/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
     /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
    / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
   /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
                               |___/

   Copyright 2018 AllThingsTalk

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/* The SGP30 sensor measures eCO2 (equivalent calculated carbon-dioxide) concentration within a range of 0 to 60,000 parts per million (ppm), 
   and TVOC (Total Volatile Organic Compound) concentration within a range of 0 to 60,000 parts per billion (ppb).
   The DHT22 sensor measures Temperature (Range -40 - 80 °C, with an accuracy of +/-0.5°C) and Humidity (Range 5% - 99% RH, with an accuracy of +/- 2% RH)
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ATT_IOT.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <CborBuilder.h>
#include "sensirion_common.h"
#include "sgp30.h"
#include "keys.h"

// Define mqtt endpoint
#define mqtt "api.allthingstalk.io"  // broker

#define DHTPIN 13       // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define SAMPLE_FREQ 60000 //sample frequency in milliseconds


//Constructors
ATTDevice device(DEVICE_ID, DEVICE_TOKEN);
CborBuilder payload(device);
DHT dht(DHTPIN, DHTTYPE);

//Setup Communication

WiFiClient espClient;
PubSubClient pubSub(mqtt, 1883, espClient);

void setup()
{
  
  //Start the serial port for debugging
  Serial.begin(115200);  // Init serial link for debugging
  Serial.println("Init sensors!");

  //Power on the Grove Sensors on Wio Link Board
  #if defined(ESP8266)
          pinMode(15,OUTPUT);
          digitalWrite(15,1);
          Serial.println("Set wio link power!");
          delay(500);
  #endif
  
  //Start the DHT sensor
  dht.begin();

  /*Init module,Reset all baseline,The initialization takes up to around 15 seconds, during which
  all APIs measuring IAQ(Indoor air quality ) output will not change.Default value is 400(ppm) for co2,0(ppb) for tvoc*/
  while (sgp_probe() != STATUS_OK) {
         Serial.println("SGP init failed");
         while(1);
         }
    
  Serial.println("ready to sample the sensors!");

  //Time to connect to the wifi
  setupWiFi(WIFI_SSID, WIFI_PASSWORD);

  // Connect to the DS IoT Cloud MQTT Broker
  Serial.println("Subscribe MQTT");

  while (!device.subscribe(pubSub)) // Subscribe to mqtt
  Serial.println("retrying");

}


void setupWiFi(const char* ssid, const char* password)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println();
  Serial.println("WiFi connected");
}

unsigned long timer; 

void loop()
{
  s16 err=0;
  u16 tvoc_ppb, co2_eq_ppm;
  unsigned long curTime = millis();
  if (curTime > (timer + SAMPLE_FREQ))    {

  // sampling data
  Serial.println("sample DHT sensor");
  float temperature = dht.readTemperature(false, true);
  float humidity = dht.readHumidity(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Gor wrong readings from DHT sensor!, retrying...");
    return;
    }
     
  Serial.println("sample gas sensor");
     
  /* start a tVOC and CO2-eq measurement and to readout the values*/
  err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
    if (err != STATUS_OK) {
      Serial.println("error reading tvoc_ppb & co2_eq_ppm values\n"); 
      }

  // Transmit the counter value to ATTALK IoT Cloud
  
  payload.reset();
  payload.map(4); //number of assets that need to be decoded. If number is not correct, parsing in the platform will not be correct.
  payload.addNumber(temperature, "t");
  payload.addInteger(int(humidity), "h");
  payload.addInteger(tvoc_ppb, "v");
  payload.addInteger(co2_eq_ppm, "c");
  payload.send();
  
  Serial.println("Transmitted data");
  Serial.print("tvoc: ");
  Serial.println(tvoc_ppb);
  Serial.print("co2_eq: ");
  Serial.println(co2_eq_ppm);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(int(humidity));
  Serial.println("");
  
  Serial.print("time = ");Serial.println(curTime);
  timer = curTime;
  }
  device.process();
}
