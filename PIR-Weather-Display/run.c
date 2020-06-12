#include <brzo_i2c.h>
#include "SSD1306Brzo.h"
#include <Adafruit_SHT31.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>


//debuging
#define SERIAL 1

//wifi settings
#define wifi_ssid "wifiname"
#define wifi_password "wifipassword"

//mqtt settings
#define mqtt_server "mqtturl"
#define mqtt_user ""  //leave empty to communicate without authentication
#define mqtt_password ""  //leave empty to communicate without authentication
#define ROUTE "devices/pir-weather/" //main MQTT topic that is used

//Sensors settings
#define pinPIR 13
#define pinTEMP 2 //D4
#define pinLIGHT A0
#define pinSHTCL 5
#define pinSHTSDA 4 
#define intervallPIR 2000 //Intervall the sensor is read in ms
#define intervallTEMP 10000 
#define intervallLIGHT 10000
#define intervallSHT 10000

//Display Settings
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 256
#define pinDISPSDA 14
#define pinDISPSCL 12
#define intervallDISP 500

//helpers
int pir_value = 0;
unsigned int pir_last = 0;

float temp_value = 0;
unsigned int temp_last = 0;

float light_value = 0;
unsigned int light_last = 0;

float sht_humidity = 0;
float sht_temperature = 0;
unsigned int sht_last = 0;

unsigned int disp_last = 0;
int disp_cnt = 0;

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWireTemp(pinTEMP); 
DallasTemperature sensorTemp(&oneWireTemp);
Adafruit_SHT31 sensorSht = Adafruit_SHT31();
SSD1306Brzo display(0x3c, pinDISPSDA, pinDISPSCL);

void setup() {
  
  
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while(!Serial) {
  }
  Serial.println("Device Started");
  Serial.println("-------------------------------------");

  setupWifi();
  client.setServer(mqtt_server, 1883);  // here you can change the port

  Serial.println("Connected to Network!");
  Serial.println("-------------------------------------");

  // Setup Pins
  pinMode(pinPIR, INPUT);
  //digitalWrite(pinPIR, LOW);
  pinMode(pinLIGHT, INPUT);
  pinMode(pinTEMP, INPUT);
  sensorTemp.begin(); 
  sensorSht.begin(0x44);
  display.init();
  display.flipScreenVertically();

  Serial.println("Finished Sensor Setup!");
  Serial.println("-------------------------------------");
}

//Enters WiFi
void setupWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Enter MQTT
void connectMqtt() {
  // Loop until we're reconnected or timeout
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(3000);
    }
  }
}

void sendMqtt(const char message[], char mqttChannel[]) {
  char mqttRoute[] = ROUTE;
  strcat(mqttRoute, mqttChannel);  
  client.publish(mqttRoute, message);
}

int readPIR (int mqtt) {
  pir_value = digitalRead(pinPIR);

  if (SERIAL == 1) {
    Serial.print("PIR Value: ");
    Serial.println(pir_value);
  }

  if (mqtt == 1) {
    sendMqtt(String(pir_value).c_str(), "MOTION");
  }

  return pir_value;
}

float readLight (int mqtt) {
  light_value = (analogRead(pinLIGHT)*100)/1024;

  if (SERIAL == 1) {
    Serial.print("Light Value: ");
    Serial.println(light_value);
  }

  if (mqtt == 1) {
    sendMqtt(String(light_value).c_str(), "LIGHT");
  }

  return light_value;
}

float readTemp (int mqtt) {
  sensorTemp.requestTemperatures();
  temp_value = sensorTemp.getTempCByIndex(0);

  if (SERIAL == 1) {
    Serial.print("Temperature1: ");
    Serial.println(temp_value);
  }

  if (mqtt == 1) {
    sendMqtt(String(temp_value).c_str(), "TEMPERATURE");
  }

  return temp_value;
}

float readSht (int mqtt) {
  delay(500);
  sht_humidity = sensorSht.readHumidity();
  delay(500);
  sht_temperature = sensorSht.readTemperature();


  if (SERIAL == 1) {
    Serial.print("Temperature (SHT): ");
    Serial.println(sht_temperature);
    Serial.print("Humidity (SHT): ");
    Serial.println(sht_humidity);
  }

  if (mqtt == 1) {
    sendMqtt(String(sht_humidity).c_str(), "HUMIDITY");
    sendMqtt(String(sht_temperature).c_str(), "TEMPERATURE2");
  }

  return sht_humidity;
}

void updateDisp(){
  display.setFont(ArialMT_Plain_16);
  display.clear();
  display.drawStringMaxWidth(1, 0, 500, "Temp: ");
  display.drawStringMaxWidth(60, 0, 500, String(temp_value).c_str());
  display.drawStringMaxWidth(100, 0, 500, "C");
  display.drawStringMaxWidth(1, 20, 500, "Hum: ");
  display.drawStringMaxWidth(60, 20, 500, String(sht_humidity).c_str());
  display.drawStringMaxWidth(100, 20, 500, "%H");
  display.drawStringMaxWidth(1, 40, 500, "Light: ");
  display.drawStringMaxWidth(60, 40, 500, String(int(light_value)).c_str());
  display.drawStringMaxWidth(110, 40, 500, "%");
  if (disp_cnt == 0) {
    disp_cnt = 1;
    display.drawLine(0,63, 127,63);
    display.drawLine(0,62, 127,62);
  } else {
    disp_cnt = 0;
  }
  display.display();
  if (pir_value == 1) {
    display.invertDisplay();
  } else {
    display.normalDisplay();
  }
}

void loop() {
  //Run MQTT
  if (!client.connected()) {
    connectMqtt();
  }

  //Timing for sensors and display
  if ((millis() - pir_last) > intervallPIR) {
    pir_last = millis();
    readPIR(1);
  }
  if ((millis() - temp_last) > intervallTEMP) {
    temp_last = millis();
    readTemp(1);
  }
  if ((millis() - light_last) > intervallLIGHT) {
    light_last = millis();
    readLight(1);
  }
  if ((millis() - sht_last) > intervallSHT) {
    sht_last = millis();
    readSht(1);
  }
  if ((millis() - disp_last) > intervallDISP) {
    disp_last = millis();
    updateDisp();
  }

}
