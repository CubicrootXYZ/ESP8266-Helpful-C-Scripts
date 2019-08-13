#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//SETTINGS
//wifi settings
#define wifi_ssid "ssid"
#define wifi_password "password"
//mqtt settings
#define mqtt_server "mqtt ip without port"
#define mqtt_user ""  //leave empty to communicate without authentication
#define mqtt_password ""  //leave empty to communicate without authentication
#define ROUTE "devices/multisensor1/" //default route where the values are published (Temperature: ROUTE/TEMPERATURE)

#define DHTPIN D2     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
char errorReport[400];

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize. Wait max 5 seconds
  //int milis = millis();
  //int timedelta = 0;
  while(!Serial) {
  }
  //error-report
  if (!Serial) {
    strcat(errorReport, "\nNot able to initialize Serial Monitor after 5 Seconds.");
  }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");

   //setup wifi and mqtt
  //setup_wifi();
  client.setServer(mqtt_server, 1883);  // here you can change the port

  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");
}


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
    strcat(errorReport, "\nCan't connect to WiFi after 10 Seconds.");
  }
}


void reconnect() {
  // Loop until we're reconnected or timeout
  int milis = millis();
  int timedelta = 0;
  while (!client.connected() || timedelta < 12000) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client", user, pw)) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
    timedelta = millis()-milis;
  }
  if (!client.connected()) {
    strcat(errorReport, "\nCan't connect to MQTT Broker");
  }
}

void pushMQTT(const char message[], char mqttChannel[]) {
  char mqttRoute[] = ROUTE;
  strcat(mqttRoute, mqttChannel);  
  client.publish(mqttRoute, message);
}

void logData() {
  // Report data

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    strcat(errorReport, "\nFailed to read from DHT22.");
    return;
  } else {  
    // Compute heat index in Fahrenheit (the default)
    float hif = dht.computeHeatIndex(f, h);
    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");

    pushMQTT(String(h).c_str(), "HUMMIDITY");
    pushMQTT(String(t).c_str(), "TEMPERATURE");
    pushMQTT(String(hic).c_str(), "HEATINDEX");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  logData();
}
