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
 
#ifndef CBOREN_H
#define CBOREN_H

#include "Arduino.h"
#include "ATT_IOT.h"

class CborBuilder {

  public:
    CborBuilder(ATTDevice &device);
    CborBuilder(ATTDevice &device, uint32_t initialCapacity);
    ~CborBuilder();

    unsigned char *getData();
    unsigned int getSize();

    void reset();
    bool send();

    // Method to construct a custom cbor payload
    void addBoolean(bool value, const String asset);
    void addInteger(int value, const String asset);
    void addNumber(double number, const String asset);
    void addString(const String value, const String asset);
    void addGps(double latitude, double longitude, double altitude, const String asset);

    void map(const unsigned int size);

    void init(unsigned int initalCapacity);
    unsigned char *buffer;
    unsigned int capacity;
    unsigned int offset;
 
  private: 
    virtual void putByte(unsigned char value);
    virtual void putBytes(const unsigned char *data, const unsigned int size);
  
    void writeTypeAndValue(uint8_t majorType, const uint32_t value);
    void writeTypeAndValue(uint8_t majorType, const uint64_t value);
    void addNumber(double number);
    
    void writeInt(const int value);
    void writeBytes(const unsigned char *data, const unsigned int size);
    void writeString(const char *data, const unsigned int size);
    void writeString(const String str);
    void writeArray(const unsigned int size);
    void writeTag(const uint32_t tag);
    void writeSpecial(const uint32_t special);
    
    ATTDevice* _device;
};

#endif