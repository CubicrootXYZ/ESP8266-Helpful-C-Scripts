# ESP8266-Helpful-C-Scripts
Helpful Scripts for the ESP8266 written in C. 

# How to use
Just copy and paste the script into the Arduino IDE and upload it to your ESP. 

I am using: 
- NodeMCU v3

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

## Complete Build: Multisensor1 

A sensor for temperature, humidity, light and pressure. Values are displayed at 2 displays. 

## Actors

**IR Receiver and Transmitter**

A IR remote and receiver that displays the received HEX codes (RC5) on a SSD1306 display (256x64). You can send RC5-HEX codes via MQTT to use the IR transmitter.  

! Make sure to only send the non-inverted HEX-codes to the esp, inverted ones are looking like 0x8** whereas non-inverted ones look like 0x**. The esp will automatically send the non-inverted one 2 times and then the inverted one 2 times, thats according to the RC5 Standard and common practise. !

[More Information on my blog](https://blog.cubicroot.xyz/hacking-a-ir-remote-with-a-esp8266)
