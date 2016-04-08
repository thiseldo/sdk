# The Things Uno SDK

## Introduction

This is the Arduino library for The Things Uno to communicate with The Things Network. You can also use this library for other Arduino-compatible devices featuring the Microchip RN2xx3 chip.

## Installation

1. Download the latest zip file from the `release` folder
2. In the Arduino IDE, go to Sketch -> Include Library -> Add .ZIP Library
3. Select `TheThingsUno.zip`
4. Verify that the Board under Tools is set to Arduino Leonardo and Port is set to the device connected to your pc

## Example

Open the `hello-world` example from File -> Examples -> TheThingsUno, or paste the following code in a new sketch:

```Arduino
#define DEBUG

#include "TheThingsUno.h"

// Set your device address
const byte devAddr[4] = {0x02, 0xDE, 0xAE, 0x00};

// Set your NwkSKey and AppSKey
const byte nwkSKey[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
const byte appSKey[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};

#define debugSerial Serial
#define loraSerial Serial1

TheThingsUno ttu;

void setup()
{
  debugSerial.begin(115200);
  loraSerial.begin(57600);

  delay(3000);
  debugSerial.println("Initializing...");

  ttu.init(loraSerial, debugSerial);
  ttu.reset();
  ttu.personalize(devAddr, nwkSKey, appSKey);
  ttu.showStatus();

  debugSerial.println("Setup for The Things Network.");

  delay(1000);
}

void loop() {
  ttu.sendString("Hello world!");

  delay(5000);
}
```

Click Verify to compile the sketch. Click Upload to upload the example to your The Things Uno.

## API

TheThingsUno class features the following public methods:

```C++
  public:
    void init(Stream& modemStream, Stream& debugStream);
    void reset(bool adr = true, int fsb = DEFAULT_FSB);
    bool personalize(const byte devAddr[4], const byte nwkSKey[16], const byte appSKey[16]);
    bool join(const byte appEui[8], const byte appKey[16]);
    void sendBytes(const byte* buffer, int length, int port = 1, bool confirm = false);
    void sendString(String message, int port = 1, bool confirm = false);
    void showStatus();
}
```
