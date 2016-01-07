@mainpage

This is a C++ Library for reading / writing email messages from an Arduino. The Arduino is connected via WIFI (the ESP8266 chip), and must have a decent amount of memory (for eg. the Mega).

This library depends on the PolygonDoorESP8266 library: https://github.com/polygondoor/PolygonDoor_ESP8266

# Source 

Source can be download at <https://github.com/polygondoor/HackySoc>

# How to get started

First, please download and install PolygonDoorESP8266.

Visit <http://polygondoor.com.au/2016/01/06/adding-wifi-to-the-arduino/> to see how to wire up (and power) the ESP8266 chip on an Arduino.


# API List

    bool connectToAP(String ssid, String pwd);

    bool sendMessage(String recipient, String subject, String body);

    int  countInbox(void);

    bool getNewMessage(void);


# Mainboard Requires

  - RAM: not less than 2KBytes
  - Serial: one serial (HardwareSerial or SoftwareSerial) at least 

# Suppported Mainboards
  
  - Arduino MEGA and its derivatives


-------------------------------------------------------------------------------

# The End!

-------------------------------------------------------------------------------
