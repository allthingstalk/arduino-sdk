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

#ifndef ATTDevice_h
#define ATTDevice_h

//#include "Arduino.h"
#include <Client.h>
#include <PubSubClient.h>
#include <string.h>
#include <EthernetClient.h>

// this class represents the ATT cloud platform.
class ATTDevice
{
  public:
    /**
     * Create the object, using the credentials of our device.
     */
    //ATTDevice();
    ATTDevice(String deviceId, String deviceToken);

    void setCredentials(String deviceId, String token);

    /**
     * Connect with the http server (call first)
     *
     * @param Client the client object to use for communicating with the cloud HTTP server (this is usually an EthernetClient, WifiClient or similar)
     * @param httpServer the name of the http server to use, kept in memory until after calling 'Subscribe'
     *
     * @return true when subscribe was successful, otherwise false
     */
    bool connect(Client* httpClient, const char httpServer[]);

    /**
     * Create or update the specified asset.
     *
     * > After this call, the name will be in lower case, so that it can be used to compare with the topic of incomming messages.
     */
    void addAsset(String name, String title, String description, String assetType, String dataType);

    /**
     * Stop http processing & make certain that we can receive data from the mqtt server.
     *
     * @return true when successful, false otherwise
     */
    bool subscribe(PubSubClient& mqttclient);

    /**
     * Stop http processing & make certain that we can receive data from the mqtt server, given the specified username and pwd.
     *
     * @return true when successful, false otherwise
     */
    bool subscribe(PubSubClient& mqttclient, const char* username);

    /**
     * Send a data value to the cloud server for the sensor with the specified id.
     */
    void send(String value, String asset);

    /**
     * Closes any open connections (http & mqtt) and resets the device. After this call, you can call connect and/or subscribe again. Credentials remain stored.
     *
     * > All clients (httpclient & pubsubClient) are the caller's responsibility to clean up.
     */
    void close();

    /**
     * Check for any new mqtt messages.
     */
    bool process();

    /**
     * @return the asset name found in the topic
     */
    String getAssetName(char* topic, int topicLength);

    // Send binary payload
    bool sendBinary(void* packet, unsigned char size);

    // Send cbor payload
    bool sendCbor(unsigned char* data, unsigned int size);

  private:
    String _serverName;  // store the name of the http server that we should use
    String _token;       // the client key provided by the user
    Client* _client;     // raw http communication. Possible to save some memory here: pass the client as a param in connect, put the object local in the setup function

    const char* _mqttUserName;  // we store a local copy of the the mqtt username and pwd, so we can auto reconnect if the connection was lost
    const char* _mqttpwd;

    /**
     * Subscribe to the mqtt topic so we can receive data from the server.
     */
    void mqttSubscribe();

    /**
     * Read all the data from the Ethernet card and display on the debug screen.
     */
    void getHTTPResult();

    /**
     * Build the content that has to be sent to the cloud using mqtt (either a csv value or a json string).
     */
    char* buildJsonContent(String value);

    /**
     * Close the http connection, if any.
     */
    void closeHTTP();

    /**
     * Close the mqtt connection, if any.
     */
    void closeMQTT();

    PubSubClient* _mqttclient;  // provides mqtt support

    /**
     * Try to create a connection with the mqtt broker. also used to try and reconnect.
     */
    bool mqttConnect();  // so inheriters can reconnect with the mqtt server if they detect a network loss
    String _deviceId;    // the device id provided by the user.
    String _clientId;    // the client id provided by the user.
};

#endif
