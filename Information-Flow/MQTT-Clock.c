//just a simple script that allows you to run a clock at an ESP, the time is set via mqtt, just send a HH:MM string and the ESP will readjust himself

#include <ESP8266WiFi.h>  //wifi lib
#include <PubSubClient.h> //mqtt lib
#include <string.h> //lib for string manipulation
#include <time.h> //lib for handling time (I think it isnt needed anymore)

//WIFI SETTINGS
#define wifi_ssid "ssid"
#define wifi_password "password"

//MQTT Settings, leave pw and user blank if not needed
#define mqtt_server "IP Adress of the MQTT Server"
#define mqtt_port 1883
#define mqtt_user ""
#define mqtt_password ""

//topic the time is sent to
#define time_topic "test"

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long int seconds, minutes, hours;
unsigned long int globalRuntime;
unsigned long int milliseconds;
String pload = "";

// THE SETUP
void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while(!Serial) { }

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
}

//HELPER FUNCTION
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

//UPDATES THE TIME
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

//SETUP THE WIFI
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
      client.subscribe(time_topic);
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
  //Connect to the MQTT Server
  if (!client.connected()) {
    Serial.println("Reconnect");
    reconnect();
  }
  client.loop();

  // prints out the time
  if (millis()%1000 == 0) {
      refreshTime();
    
      Serial.print(hours);
      Serial.print(':');
      Serial.print(minutes);
      Serial.print(':');
      Serial.println(seconds);
  }

}
