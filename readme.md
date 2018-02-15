Arduino SDK
---

### Hardware

This SDK is for Arduino and Arduino compatible boards, such as
- Arduino Leonardo
- Arduino Ethernet
- LinkitOne `http://download.labs.mediatek.com/package_mtk_linkit_index.json`
- NodeMCU `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

### Installation

Download the source code and unzip and copy the content to your arduino libraries folder (usually found at `/libraries`) _or_ import the library folders directly using the Arduino IDE under Sketch > Include Library > Add .ZIP library (repeat this step for each of the libraries: *ATT_IOT*, *pubsubclient* and _ArduinoJson_)

### Device credentials

You can either set them **globally**, using the same credentials for all sketches using the sdk.<br>
Or you can set them **locally** in a specific sketch, overriding the global settings.

You can find these credentials under your device at AllThingsTalk in the _SETTINGS > Authentication_ tab.

Depending on how you initialize the device object in your sketch, the global or local credentials will be used.

* `ATT_IOT device("your_device_id", "your_device_token");` will use the provided local credentials.
* `ATT_IOT device;` will use the global credentials from the **keys.h** file

> Open the `keys.h` file on your computer and enter your _deviceid_ and _devicetoken_.

```
/****
 * Enter your AllThingsTalk device credentials below
 */
#ifndef KEYS_h
#define KEYS_h

const char* DEVICE_ID = "your_device_id";
const char* DEVICE_TOKEN = "your_device_token";

#endif
```

### Payloads and sending data

There are three ways to send your data to AllThingsTalk

* `Standard json`
* `Cbor payload`
* `Binary payload`

Standard json will send a single datapoint to a single asset. Both _Cbor_ and _Binary_ allow you to construct your own payload. The former is slightly larger in size, the latter requires a small decoding file [(example)](https://github.com/allthingstalk/arduino-nbiot-sdk/blob/master/examples/counter/nbiot-counter-payload-definition.json) on the receiving end.

#### Single asset

Send a single datapoint to a single asset using the `send(value, asset)` functions. Value can be any primitive type `integer`, `float`, `boolean` or `String`. For example

```
ATT_IOT device;
```
```
  device.send(String(25), "counter");
```

#### Cbor

```
ATT_IOT device;
CborBuilder payload(device);  // Construct a payload object
```
```
  payload.reset();
  payload.map(1);  // Set number of datapoints in payload
  payload.addInteger(25, "counter");
  payload.send();
```

#### Binary payload

Using the [AllThingsTalk ABCL language](http://docs.allthingstalk.com/developers/custom-payload-conversion/), you can send a binary string containing datapoints of multiple assets in a single message. The example below shows how you can easily construct and send your own custom payload.

> Make sure you set the correct decoding file at AllThingsTalk. Please check our documentation and the included experiments for examples.

```
ATT_IOT device;
PayloadBuilder payload(device);  // Construct a payload object
```
```
  payload.reset();
  payload.addInteger(25);
  payload.addNumber(false);
  payload.addNumber(3.1415926);
  payload.send();
```

### Examples

All examples can be found in the `ATT_IOT/examples` folder.

#### Arduino Ethernet / Leonardo

> Depending on the board, make sure you use the correct Ethernet library in your sketch.
* Arduino Ethernet `#include <Ethernet.h>`
* Arduino Leonardo `#include <Ethernet2.h>`

* Basic examples for sensing and actuation
  * `counter.ino` send data from your Arduino to AllThingsTalk _(sensing)_
  * `led-actuation.ino` toggle a led on your Arduino from AllThingsTalk _(actuation)_
* Rapid Development Kit examples
  * `light-sensor.ino` Measure light in your environment
  * `motion-sensor.ino` Turn on LED and get a notification when movement is detected
  * `smart-doorbell` Get notified when someone is at the door

#### NodeMCU

* `nodemcu.ino` will show you how to send data from the NodeMCU to AllThingsTalk as well as the other way around, actuating the onboard LED from the cloud.

> Make sure you fill in your network credentials in the `setup()` method of the sketch
```
setupWiFi("your_wifi_ssid", "your_wifi_password");  // Connect to the WiFi network
```

#### Linkit One

* `counter.ino`
* `gps.ino`

Please take note of the following

> In the **ATT_IOT.h** file, comment the following line `// #include <EthernetClient.h>`

> Newer versions of the Arduino IDE may conflict with the Linkit One. [Arduino IDE version 1.6.5](https://www.arduino.cc/en/Main/OldSoftwareReleases#previous) is tested and will work with the examples below.