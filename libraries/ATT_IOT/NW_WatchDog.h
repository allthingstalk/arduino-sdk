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

#ifndef ATT_NW_WatchDog_h
#define ATT_NW_WatchDog_h

#include <ATT_IOT.h>
#include <string.h>
#include <PubSubClient.h>

/*
  Adds a network watchdog feature to the att device. 
  When the device made a successfull connection with the cloud, it will automatically add an actuator to the device, with id -1, which is used to verify network connectivity.
  Warning: uses asset id -1  for the watchdog actuator.
*/
class NW_WatchDog
{
  public:
    /**
     * Create the object
     */
    NW_WatchDog(PubSubClient& mqttclient, const char* deviceId, const char* clientId, unsigned int frequency = 300000);

    /**
     * Send a ping to the broker
     */
    void Ping();
  
    /**
     * Check if we need to resend a ping and if we received the previous ping in time.
     *
     * @return true if the previous ping was in time, else false is returned. You can then try to recreate the connection with the broker
     */
    bool CheckPing();
    
    /**
     * Check if we received a ping back from the broker.If not, the broker connection will be closed (and reopened upon the next call to 'process'.
     *
     * @return true if the pin was for the network monitor actuator. Otherwise, it returns false
     */
    bool IsWatchDog(int pinNr, String& value);
  
  private:  
    unsigned long _nextPingAt = 0;    // time when the next ping should be sent
    unsigned int _pingCounter = 0;    // next ping to send out
    unsigned long _lastReceived = 0;  // ping that we last received, set to same as first ping, otherwise the first call will fail
    PubSubClient* _mqttclient;
    const char* _deviceId;
    const char* _clientId;
    unsigned char _topicLen;  // so we only have to calculate it 1 time
    unsigned int _frequency;
    
};

#endif