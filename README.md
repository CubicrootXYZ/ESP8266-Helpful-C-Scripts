# ESP8266-Helpful-C-Scripts
Helpful Scripts for the ESP8266 written in C. 

Licensed under CC-BY-NC.

# How to use
Just copy and paste the script into the Arduino IDE and upload it to your ESP. 

I am using: 
- NodeMCU v3
- Wemos D1 Mini

# Available Scripts
## Information Flow
**MQTT Clock**

A clock running on your ESP thats beeing set via a MQTT-Message. 

## Lights

**MQTT LED Stripe with Temperature Control**

Running a 12 V LED Stripe (5050) via MQTT (prepared for OpenHAB). Light will shut down if to hot.

**MQTT LED Stripe**

Running a 12 V LED Stripe (5050) via MQTT (prepared for OpenHAB).

## Display

**Clock Weather**

Displaying the time (set via mqtt) and the local temperature and humidity (DHT22-Sensor) to a SSD1306-Display

**NTP time Home Assistant Weather**

SSD1306 with local time and date from NTP server and weather from home assistant. 

## Complete Build: Multisensor1 

A sensor for temperature, humidity, light and pressure. Values are displayed at 2 displays. 

## PIR-Weather-Display

A build that includes a SHT temperature and humidity sensor, a Dallas 18B20 temperature sensor, a PIR-Motion, a Light sensor sensor and a SSD1306 display.

Build sketch is included.

## Actors

**IR Receiver and Transmitter**

A IR remote and receiver that displays the received HEX codes (RC5) on a SSD1306 display (256x64). You can send RC5-HEX codes via MQTT to use the IR transmitter.  

! Make sure to only send the non-inverted HEX-codes to the esp, inverted ones are looking like 0x8** whereas non-inverted ones look like 0x**. The esp will automatically send the non-inverted one 2 times and then the inverted one 2 times, thats according to the RC5 Standard and common practise. !

[More Information on my blog](https://blog.cubicroot.xyz/hacking-a-ir-remote-with-a-esp8266)

**MQTT Audioswitch**

A Switch that can change between 4 audio channels. Simple 3.5mm audio jacks. 

[More Information on my blog](https://blog.cubicroot.xyz/mqtt-managed-audio-switch)
