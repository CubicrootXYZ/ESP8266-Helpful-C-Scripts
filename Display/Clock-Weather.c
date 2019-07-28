//with this script you can Display a Clock (set via mqtt) and the data from a DHT22-Sensor on a SSD1306 Display
// !! IMPORTANT !! Do NOT use delay(), this will result in some time offsets

//How to setup your pins for a NodeMCU: 
//Display: SDA => D1, SCK => D2
//DHT22 => D3
//or just change the pins below
           
#include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Brzo.h"  //lib for the display
#include "DHT.h"  //lib for the DHT22 sensor
#include <ESP8266WiFi.h>  //wifi lib
#include <PubSubClient.h> //mqtt lib
#include <string.h> //lib for string manipulation
#include <time.h> //lib for handling time (I think it isnt needed anymore)

// Initialize the OLED display using brzo_i2c
SSD1306Brzo display(0x3c, 5, 4);
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 256

// Initialize the DHT22
#define DHTPIN D3    
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

//setup the WiFi
#define wifi_ssid "ssid"
#define wifi_password "passwort"
WiFiClient espClient;

//setup mqtt, leave user and pw empty if not set
#define mqtt_server "IP from your MQTT Server"
#define mqtt_user ""
#define mqtt_password ""
PubSubClient client(espClient);

//setup the mqtt-topics for receiving time (HH:MM String) and pushing DHT-Data (ROUTE + HUMIDITY/TEMPERATURE)
#define time_topic "test"
#define ROUTE "yourroute/"

//just soime helpers
String pload = "";
int loopswitch = 0;
int ambientlight = 0;
char errorReport[400];
int j = 0;

unsigned long int seconds, minutes, hours;
unsigned long int globalRuntime;
unsigned long int milliseconds;

//SETUP
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while(!Serial) { }

  setup_wifi();
  
  display.init();
  // display.flipScreenVertically();  //use this if your screen is flipped

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
}

// HELPER for getting mqtt messages
String getValue(String data, char separator, int index)
{
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

//TIME UPDATE
void refreshTime() {
  unsigned long int localRuntime = millis();
  unsigned long int timegone = localRuntime - globalRuntime;

  seconds += (timegone/1000);

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

//MQTT PUSH
void pushMQTT(const char message[], char mqttChannel[]) {
  char mqttRoute[] = ROUTE;
  strcat(mqttRoute, mqttChannel);  
  client.publish(mqttRoute, message);
}

//SETUPP WiFi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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

// MQTT RECONNECT
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe("information/time/time_local_hhmm");
    }
  }
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

// LOOP
void loop() {

  //make sure client is connected to the mqtt server
  if (!client.connected()) {
    Serial.println("Reconnect");
    reconnect();
  }
  client.loop();

  //setup display
  display.setContrast(100);  
  display.setFont(ArialMT_Plain_24);

  //show IP on startup, delete if not needed
  if (j == 0) {
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawStringMaxWidth(64, 30, 140, WiFi.localIP().toString());
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.display();
    delay(4000);
    display.clear();
    j = 1;
  }
  
  // Read DHT
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Update temperature and humidity ever minute
  if (millis()%60000 == 0) {
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      strcat(errorReport, "\nFailed to read from DHT22.");
      return;
    } else {  
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);

      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print(" *C\t");
      Serial.print("Heat index: ");
      Serial.print(hic);
      Serial.println(" *C ");  

      //push temperature and humidity to mqtt server
      pushMQTT(String(h).c_str(), "HUMMIDITY");
      pushMQTT(String(t).c_str(), "TEMPERATURE");
    }
      // read light sensor
      ambientlight = (analogRead(A0)*100)/1023;
      pushMQTT(String(ambientlight).c_str(), "AMBIENTLIGHT");
  }
  
    // Update the clock every 1 Second
    if (millis()%1000 == 0) {
      unsigned long int starttime = millis();
      refreshTime();
    
      Serial.print(hours);
      Serial.print(':');
      Serial.print(minutes);
      Serial.print('.');
      Serial.println(seconds);
      
      display.clear();
      display.setFont(ArialMT_Plain_24);

      char minutes_str[12];
      sprintf(minutes_str, "%02d", minutes);
      char hours_str[12];
      sprintf(hours_str, "%02d", hours);
      char ambientlight_str[12];
      sprintf(ambientlight_str, "%3d", ambientlight);
      
      display.drawStringMaxWidth(0, 0, 500, hours_str);
      if (loopswitch == 1) {
        display.drawStringMaxWidth(25, 0, 500, ":");
      }
      display.drawStringMaxWidth(30, 0, 500, minutes_str);

      display.setFont(ArialMT_Plain_16);
      display.drawStringMaxWidth(0, 30, 500, String(t)); 
      display.drawStringMaxWidth(40, 30, 500, "C");  
      display.drawStringMaxWidth(60, 30, 500, String(h)); 
      display.drawStringMaxWidth(100, 30, 500, "%");  
      display.drawStringMaxWidth(80, 5, 500, ambientlight_str); 
      display.drawStringMaxWidth(105, 5, 500, "%"); 
      display.display();
      
      if (loopswitch == 0) {
        loopswitch = 1;
      } else {
        loopswitch = 0;
      }
  }

}
