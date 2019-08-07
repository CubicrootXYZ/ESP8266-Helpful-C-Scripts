#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Brzo.h"  //lib for the display
#include <string.h> //lib for string manipulation
#include <math.h>

/* displaying time and sensor data on a ssd1306 and a 7 segment 4 digit display with a NodeMCU
 * I am using a BME280 with SPI and a TEMT6000 light sensor
 * Everything is powered via the 3.3 Volt Pins from the NodeMCU
 * 
 * Wiring:
 * A0 => Ambient Light Sensor (S)  
 * D1 => SSD1306 SCL 
 * D2 => SsD1306 SDA
 * D0 => BME280 CSB/CS
 * D3 => BME280 SCL/SCK
 * D4 => BME280 SDA/MOSI
 * D5 => BME280 SDO/MISO
 * D6 => 4Digit CLK
 * D7 => 4Digit DIO
 * D8 => ! Do not use for SPI/I2C connection, not working !
 */

// Set all Pins
#define PINlight A0 //Pin for the ambientl ight Sensor, you also can use a simple LDR

#define BME_SCK 0 //SCK Pin from the BME (Sometime labeled as SCL)
#define BME_MISO 14 //MISO Pin from the BME (Sometime labeled as SDO)
#define BME_MOSI 2 //MOSI Pin from the BME (Sometime labeled as SDA)
#define BME_CS 16 //CS Pin from the BME (Sometime labeled as CSB)
#define SEALEVELPRESSURE_HPA (1013.25)
 
#define CLK 12  //Set the CLK pin connection to the display
#define DIO 13  //Set the DIO pin connection to the display

//SSD1306 Settings
#define DISPLAY_HEIGHT 16
#define DISPLAY_WIDTH 128

//wifi settings
#define wifi_ssid "ssid"
#define wifi_password "password"

//mqtt settings
#define mqtt_server "mqtt ip"
#define mqtt_user ""  //leave empty to communicate without authentication
#define mqtt_password ""  //leave empty to communicate without authentication
#define ROUTE "devices/multisensor1/" //Overall Route, Values are published in subchannels (TEMPERATURE, HUMIDITY...)
#define TIMECHANNEL "information/time/time_local_hhmm"  //from here the clock gets its time, push every few minutes HH:MM into this channel

// Set default values 
int ambientLight = 99;
float pressure = 9999.9;
float temperature = 99.9;
float humidity = 99.9;

// Helpers
int brightness4DIGIT = 7;
unsigned long int seconds, minutes, hours;
unsigned long int globalRuntime;
unsigned long int milliseconds;
String pload = "";
int loopswitch = 0;
char temperature_s[4];
char pressure_s[4];
char humidity_s[4];
char ambientLight_s[3];
int btn = 1;

WiFiClient espClient;
PubSubClient client(espClient);
TM1637Display display4DIGIT(CLK, DIO); //set up the 4-Digit Display.
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); //set up the BME Sensor
SSD1306Brzo displaySSD1306(0x3c, 5, 4, GEOMETRY_128_32);  //adapt to your geometry (128_64 or 128_32)

 
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  int milis = millis();
  int timedelta = 0;
  while (WiFi.status() != WL_CONNECTED || timedelta < 10000) {
    delay(500);
    Serial.print(".");
    timedelta = millis()-milis;
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not Connected to WiFi");
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe("information/time/time_local_hhmm");
    }
  }
}

String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void refreshTime() {
  unsigned long int localRuntime = millis();
  unsigned long int timegone = localRuntime - globalRuntime;

  milliseconds += timegone;

  while (milliseconds >= 1000) {
    seconds += 1;
    milliseconds -= 1000; 
  }

  while (seconds >= 60) {
    minutes += 1;
    seconds -= 60;
  }

  while (minutes >= 60) {
    hours += 1;
    minutes -= 60;
  }

  if (hours >= 24) {
    hours -= 24;
  }
  
  globalRuntime = localRuntime;
}

void pushMQTT(const char message[], char mqttChannel[]) {
  char mqttRoute[] = ROUTE;
  strcat(mqttRoute, mqttChannel);  
  client.publish(mqttRoute, message);
}

// MQTT ON MESSAGE
void callback(char* topic, byte* payload, unsigned int length) {
  pload = "";
  for (int i = 0; i < length; i++) {
    pload.concat((char)payload[i]);
  }
  Serial.println(pload);


  milliseconds = 0;
  seconds = 0;
  minutes = getValue(pload, ':', 1).toInt();
  hours = getValue(pload, ':', 0).toInt();
  globalRuntime = millis();
}

void readBME() {
  temperature = bme.readTemperature();
  pushMQTT(String(temperature).c_str(), "TEMPERATURE");
  Serial.print("Temperature (°C): ");
  Serial.println(temperature);
  pressure = bme.readPressure() / 100.0F; 
  pushMQTT(String(pressure).c_str(), "PRESSURE");
  Serial.print("Pressure (hPa): ");
  Serial.println(pressure);
  humidity = bme.readHumidity();
  pushMQTT(String(humidity).c_str(), "HUMIDITY");
  Serial.print("Humidity (%): ");
  Serial.println(humidity);
}

void readLight() {
  ambientLight = (analogRead(PINlight)*100)/1024;
  brightness4DIGIT = sqrt(ambientLight)*0.41875;
  if (brightness4DIGIT > 7) {
    brightness4DIGIT = 7;
  }
  pushMQTT(String(ambientLight).c_str(), "AMBIENTLIGHT");
  Serial.print("ambient Light (%): ");
  Serial.println(ambientLight);
}

void setup() {
  // initialize serial com
  Serial.begin(9600);
  Serial.setTimeout(2000);
  
  while(!Serial) {
  }
  //error-report
  if (!Serial) {
    Serial.println("\nNot able to initialize Serial Monitor after 5 Seconds.");
  }

  Serial.println("------------------ Device Started ------------------");

  //initialize WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //initialize BME
  bool status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
  //initialize SSD1306
  displaySSD1306.init();

    Serial.println("------------------ Initialized ------------------");
}
 
void loop()
{ 
  if (!client.connected()) {
    Serial.println("Reconnect");
    reconnect();
  }
  client.loop();

  //read sensors every minute
  if (millis()%30000 == 0) {
    readBME();
    readLight();
    refreshTime();
  }

  // refresh time and display every second
  if (millis()%1000 == 0) {
      unsigned long int starttime = millis();
      refreshTime();
    
      Serial.print(hours);
      Serial.print(':');
      Serial.print(minutes);
      Serial.print('.');
      Serial.println(seconds);
      
      display4DIGIT.setBrightness(brightness4DIGIT);
      if (seconds%2 == 0) {
        display4DIGIT.showNumberDec((hours*100)+minutes, true);
      }
      else {
        display4DIGIT.showNumberDecEx((hours*100)+minutes, (0x80 >> 1), true);
      }

      if (ambientLight > 2){
        displaySSD1306.setContrast((2*ambientLight+1), (ambientLight+141));
      } else {
        displaySSD1306.setContrast(4,20,10);
      }

      displaySSD1306.clear();
      if (btn == 1) {
        snprintf(temperature_s, 5, "%.1f", temperature);
        strcat(temperature_s, "C");
        snprintf(pressure_s, 4, "%.0f", pressure);
        strcat(pressure_s, "");
        snprintf(ambientLight_s, 3, "%i", ambientLight);
        strcat(ambientLight_s, "%");
        
        
        displaySSD1306.setFont(ArialMT_Plain_16);
        displaySSD1306.setTextAlignment(TEXT_ALIGN_RIGHT);
        displaySSD1306.drawStringMaxWidth(45, 0, 64, temperature_s);
        displaySSD1306.drawStringMaxWidth(102, 0, 80, String((int)humidity));
        displaySSD1306.drawStringMaxWidth(128, 0, 32, "%H");
        displaySSD1306.drawStringMaxWidth(96, 16, 80, String((int)pressure));
        displaySSD1306.drawStringMaxWidth(128, 16, 32, "hPa");
        displaySSD1306.drawStringMaxWidth(45, 16, 64, ambientLight_s);
      }
      
      if (seconds%2 == 0) {
        displaySSD1306.setPixel(0,0);
      } 
      displaySSD1306.display();

      

      if (loopswitch == 0) {
        loopswitch = 1;
      } else {
        loopswitch = 0;
      }

      
  }

  if (digitalRead(15) == HIGH) {
        if (btn == 0) {
          btn = 1;
        } else {
          btn = 0;
        }
      }

}
