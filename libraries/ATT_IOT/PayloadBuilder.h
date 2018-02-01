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

#ifndef PayloadBuilder_h
#define PayloadBuilder_h

#include <Arduino.h>
#include <ATT_IOT.h>

// Data type + size in bytes
#define ATTALK_BOOLEAN_SIZE 1
#define ATTALK_INTEGER_SIZE 2
#define ATTALK_NUMBER_SIZE  4
#define ATTALK_BYTE_SIZE    1
#define ATTALK_GPS_SIZE     12
#define ATTALK_ACCEL_SIZE   12


// AllThingsTalk Payload Builder
class PayloadBuilder {
  
  public:
  
    // Constructors
    PayloadBuilder(ATTDevice &device);
    PayloadBuilder(ATTDevice &device, uint8_t size);
    ~PayloadBuilder();

    /**
     * Reset the payload, to call before building a frame payload.
     */
    void reset(void);
    
    /**
     * Send the payload.
     */
    bool send(void* packet, unsigned char size);

    /**
     * Copy and send the payload.
     */
    bool send();
    
    /**
     * Return the current size of the payload.
     */
    uint8_t getSize(void);

    /**
     * Return the payload buffer.
     */
    uint8_t* getBuffer(void);

    /**
     * Copy the payload buffer.
     */
    uint8_t copy(uint8_t* buffer);

    /**
     * Add a Boolean to the payload buffer.
     *
     * @param value can be 0 or 1, represents the boolean
     *
     * @return size of the payload or 0 when max payload size has been exceeded
     */
    uint8_t addBoolean(uint8_t value);

    /**
     * Add an Integer to the payload buffer.
     *
     * @param value can be a range of -32,768 to 32,767 (2 bytes) value
     *
     * @return length of the payload or 0 when max payload size has been exceeded
     */
    uint8_t addInteger(int value);

    /**
     * Add a Number (Float) to the payload buffer.
     *
     * @param value can be a range between  3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     *
     * @return length of the payload or 0 when max payload size has been exceeded
     */
    uint8_t addNumber(float value);

    /**
     * Add a GPS object to the payload buffer.
     *
     * @param latitude :float: can be a range between 3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     * @param longitude :float: can be a range between 3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     * @param altitude :float: can be a range between 3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     *
     * @return length of the payload or 0 when max payload size has been exceeded
     */
    uint8_t addGPS(float latitude, float longitude, float altitude);

    /**
     * Add an addAccelerometer object to the payload buffer.
     *
     * @param x :float: can be a range between 3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     * @param y :float: can be a range between 3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     * @param z :float: can be a range between 3.4028235E+38 and -3.4028235E+38 (4 bytes) value
     *
     * @return length of the payload or 0 when max payload size has been exceeded
     */
    uint8_t addAccelerometer(float x, float y, float z);

  protected:
    ATTDevice* _device;
    
  private:

    uint8_t *buffer;
    uint8_t maxsize;
    uint8_t cursor;
    uint8_t sendBuffer[51];
};

#endif