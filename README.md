Arduino SDK
---

### Hardware

This SDK is for Arduino and Arduino compatible boards, such as
- Arduino Leonardo
- Arduino Ethernet
- LinkitOne
- NodeMCU

### Installation

Download the source code and unzip and copy the content to your arduino libraries folder (usually found at `/libraries`) _or_ import the .zip file directly using the Arduino IDE under Sketch > Include Library > Add .ZIP library

### Device credentials

You can either set them globally, using the same credentials for all sketches using the sdk.
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

### Sending data

### Examples

> Depending on the board, make sure you use the correct Ethernet library in your sketch.
* Arduino Ethernet `#include <Ethernet.h>`
* Arduino Leonardo `#include <Ethernet2.h>`

Two basic examples are found in the `/libraries/ATT_IOT/examples` folder.
* `counter.ino` send data from your Arduino to AllThingsTalk _(sensing)_
* `led-actuation.ino` toggle a led on your Arduino from AllThingsTalk _(actuation)_