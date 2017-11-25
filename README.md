# ESP8266 MQTT Publisher [![Build Status](https://travis-ci.org/t04glovern/esp8266-mqtt-publish.svg?branch=master)](https://travis-ci.org/t04glovern/esp8266-mqtt-publish)

## Setup

Edit the `src/main.h` file with the relevant information for your project and save it.

```cpp
#ifndef MAIN_H

// Wifi Details
const char *ssid = "YourWifiSSID";
const char *password = "YourWifiPassword";

const String thing_id = "YourThingID";

// MQTT Details
char *mqtt_server = "m14.cloudmqtt.com";
char *mqtt_server_client_id = "esp8266-device-01";
char *mqtt_server_user = "CloudMQTTUser";
char *mqtt_server_pass = "CloudMQTTPass";
int mqtt_server_port = 18583;

char *mqtt_thing_topic_pub = "accelerometer_out";
char *mqtt_thing_topic_sub = "feedback_in";

#endif

```


## MQTT Providers

### CloudMQTT

You can sign up for a free account [HERE](https://api.cloudmqtt.com)

## Platform IO

This project is build and run with PlatformIO. The library dependencies can be found in the `platformio.ini` file. Below is the current configuration targetting the ESP8266 development board. This can be changed to any variable of the ESP8266 chip.

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino

lib_deps =
    Adafruit MMA8451 Library@1.0.3
    ArduinoJson@5.11.2
    NTPClient@3.1.0
    arduinoFFT@1.2.3
    PubSubClient@2.6
```

### lib dependencies

#### Filters

A Filtering library that is used to create a single pole low pass filter to minimise the effect of noise at higher frequencies than our region of interest (1Hz-8Hz).

