# Complete Build: Multisensor1

## What this build is able to do
With this build you can monitor the following values (they are published to MQTT):
- Temperature
- Pressure
- Humidity
- Lighintensity

This build contains 1 SSD1306 display and one 4 digit 7 segment display. There following things are displayed:
- Clock (4 digit)
- Temperature (SSD1306)
- Humidity (SSD1306)
- Pressure (SSD1306)
- Lightintensity (SSD1306)

Technically the used sensor also is able of calculating your heigh, but thats nothing I needed. 

Brightness of the displays is set according to environmental illumination. By pressing the installed button the SSD1306, which displays the environmental data will shut down. So you can hide that if not needed.

## What do you need?
- 1 NodeMCU (I used an older Version, just make sure it has at least 1 analog PIN and 8 digital pins)
- 1 BME280, SPI based (I used one that is capable of doing both, I2C and SPI, make sure it runs with SPI)
- 1 TEMT6000 (You can use every other analog Sensor)
- 1 SSD1306 driven display (I am using a single color one with 128x32 pixels, make sure it runs with I2C)
- 1 TM1637 driven 4 digit 7 segment display (It looks much better if it has a colon in the middle)
- 1 Button
- Cables
- soldering accessories (or you can use a breadboard)
- Micro-USB-cable

## Installation
### Wiring
* A0 => Ambient Light Sensor (S)  
* D1 => SSD1306 SCL 
* D2 => SSD1306 SDA
* D0 => BME280 CSB/CS
* D3 => BME280 SCL/SCK
* D4 => BME280 SDA/MOSI
* D5 => BME280 SDO/MISO
* D6 => 4Digit CLK
* D7 => 4Digit DIO
* D8 => Button => VCC

It all fits on a 5x7cm board, but thats very tight. 

### Installation
1. Make sure you have a MQTT server running (Thats needed for the clock).
2. Let [this](https://github.com/CubicrootXYZ/Helpful-MQTT-Python-Scripts/blob/master/Clock/clock.py) run as a cronjob every 1 to 10 minutes. It will publish the actual time to mqtt. Don't forget to set your MQTT IP-Adress.
3. Solder everything together.
4. Open the run.c and check if all Pins are connected properly. 
5. Change the wifi settings to your wifi. Do the same with the MQTT settings in the run.c.
6- Flash the NodeMCU
7. Be happy and watch your sensor 

You might want to install something like my [MQTT-SQL-Mirror](https://github.com/CubicrootXYZ/Helpful-MQTT-Python-Scripts/tree/master/MQTT-SQL-Mirror) to be able to save the sensor data and display it later (e.g. in grafana).

## Known Issues
**The SSD1306 might act weird from time to time**

It is possible that the SSD1306 will randomly freeze or display random stuff. One reason might be, that your RAM is to small. Just reset the NodeMCU and it should work again. 

I had this issues the first few days, but now it is running for weeks without. 

**One pixel is always blinking**

That's no bug, that's a feature. So you know if the display freezed. 

**A Sensor/display is not working**

With the NodeMCU that I used pin 8 made issues when using for SPI or I2C. Also make sure that you connected ground and VCC for each part correctly. 

If nothing works out, you can test each part of the build seperately. 
