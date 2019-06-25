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
