//control a 5050 LED Stripe via MQTT, just send a RGB Value (e.g. "255,0,16") to the ESP. Used IRLZ44N MOSFETs for the Build.

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//SETTINGS
#define PINGREEN 4
#define PINRED 5
#define PINBLUE 0
#define mqtt_topic "ledstripe"
#define mqtt_port 1883

const char* ssid = "ssid";
const char* password = "password";
const char* mqtt_server = "IP of your MQTT Server";

//HELPERS
String pload = "";
int red = 0;
int green = 0;
int blue = 0;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(PINDHT, DHTTYPE);

//SETUP WIFI
void setup_wifi() {
  delay(2);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  randomSeed(micros());
}

//HELPER FUNCTION TO GET BETTER COLOR ACCURACY (NONLINEAR MODELL)
int calcColor(int color) {
  color = 0.00392156863 * (color*color);

  if (color > 255) {
    color = 255;
  } else if (color < 0) {
    color = 0;
  }

  return (int) color;
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

//MQTT ON MESSAGE
void callback(char* topic, byte* payload, unsigned int length) {
  pload = "";
  for (int i = 0; i < length; i++) {
    pload.concat((char)payload[i]);
  }
  Serial.println(pload);
  int red = getValue(pload, ',', 0).toInt();
  int green = getValue(pload, ',', 1).toInt();
  int blue = getValue(pload, ',', 2).toInt();
  red = calcColor(red);
  blue = calcColor(blue);
  green = calcColor(green);

  Serial.println(red);
  Serial.println(green);
  Serial.println(blue);
  analogWrite(PINRED, red);
  analogWrite(PINGREEN, green);
  analogWrite(PINBLUE, blue);

}

//MQTT RECONNECT
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(mqtt_topic);
    }
  }
}

//SETUP
void setup() {
  Serial.begin(9600);
  Serial.println("setup_wifi");
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(PINRED, OUTPUT);
  pinMode(PINGREEN, OUTPUT);
  pinMode(PINBLUE, OUTPUT);
  //sets duty range from 0-1023 to 0-255
  analogWriteRange(255);
}

//LOOP
void loop() {
  //connect to MQTT-Server
  if (!client.connected()) {
    Serial.println("Reconnect");
    reconnect();
  }
  client.loop();
  delay(500);
}
