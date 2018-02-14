Arduino SDK
---

### Hardware

This SDK is for Arduino and Arduino compatible boards, such as
- Arduino Leonardo
- Arduino Ethernet
- LinkitOne
- NodeMCU

### Installation

Download the source code and unzip and copy the content to your arduino libraries folder (usually found at `/libraries`) _or_ import the library folders directly using the Arduino IDE under Sketch > Include Library > Add .ZIP library (repeat this step for each of the libraries: *ATT_IOT*, *pubsubclient* and _ArduinoJson_)

### Device credentials

You can either set them globally, using the same credentials for all sketches using the sdk.<br>
Or you can set them locally in a specific sketch, overriding the global settings.

> You find your *device_id* and *device_token* under the **SETTINGS > Authentication** tab of your device in AllThingsTalk.

#### Global

Open the `keys.h` file on your computer and fill in your *device_id* and *device_token*

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

#### Local

Simply add this line at the start of your `setup()` method.

```
  device.setCredentials("your_device_id", "your_device_token");
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

#### Arduino Ethernet / Leonardo

> Depending on the board, make sure you use the correct Ethernet library in your sketch.
* Arduino Ethernet `#include <Ethernet.h>`
* Arduino Leonardo `#include <Ethernet2.h>`

Two basic examples are found in the `/libraries/ATT_IOT/examples` folder.
* `counter.ino` send data from your Arduino to AllThingsTalk _(sensing)_
* `led-actuation.ino` toggle a led on your Arduino from AllThingsTalk _(actuation)_

Three experiments from the Arduino Rapid Development kit
* `light-sensor.ino` Measure light in your environment
* `motion-sensor.ino` Turn on LED and get a notification when movement is detected
* `smart-doorbell` Get notified when someone is at the door

#### NodeMCU

`nodemcu.ino` will show you how to send data from the NodeMCU to AllThingsTalk as well as the other way around, actuating the onboard LED from the cloud.

> Make sure you fill in your network credentials in the `setup()` method of the sketch
```
setupWiFi("your_wifi_ssid", "your_wifi_password");  // Connect to the WiFi network
```