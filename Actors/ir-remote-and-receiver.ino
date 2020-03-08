#include <string.h> //lib for string manipulation
#include <time.h> //lib for handling time (I think it isnt needed anymore)
#include <brzo_i2c.h> // i2c lib
#include "SSD1306Brzo.h"
#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <Int64String.h>
#include <ESP8266WiFi.h>  //wifi lib
#include <PubSubClient.h> //mqtt lib
#include <stdio.h>
#include <stdlib.h>

#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 256
SSD1306Brzo display(0x3c, 5, 4);  // Use Pins 5 and 4 for connecting the SSD1306 Display
const uint16_t kRecvPin = 14; // IR Receiver PIN
const uint16_t kIrLed = 12; // IR Transmitter PIN
const uint16_t kCaptureBufferSize = 1024;
const uint16_t kMinUnknownSize = 12;  // Receiver message length
#define LEGACY_TIMING_INFO false

#if DECODE_AC
const uint8_t kTimeout = 50;  // Timeout between bursts, change if signals arent received
#else   // DECODE_AC

const uint8_t kTimeout = 15;
#endif  // DECODE_AC

#define mqtt_server "127.0.0.1" // MQTT IP
#define mqtt_port 1883  //MQTT Port
#define mqtt_user ""  
#define mqtt_password ""
#define SUBTOPIC "devices/ir1/send/#"  // MQTT Channel where you can send IR commands to 
#define PUBLISHTOPIC "devices/ir1/" // Main channel where the tool communicates through, Subchannel "log" is used for logging

//WiFi Settings
#define wifi_ssid "ssid"  // WiFi SSID
#define wifi_password "pswrd" // WiFi Password

WiFiClient espClient;
PubSubClient client(espClient);

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
IRsend irsend(kIrLed);
decode_results results;  // Somewhere to store the results

// Storage for displaying the last 5 received Signals
String line0 = "";
String line1 = "";
String line2 = "";
String line3 = "";
String line4 = "";
String pload = "";

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("");
  Serial.print("WiFi connected");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP().toString());
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while(!Serial) { }
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  #if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
  #endif  // DECODE_HASH
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin();

  display.init();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 500, "Point remote to");
  display.drawStringMaxWidth(0, 30, 500, "the receiver!");
  display.display();
  delay(200);
}

// MQTT RECONNECT
void mqttReconnect() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(SUBTOPIC);
    }
  }
  mqttPublish("WiFi & MQTT connected", "log");
  mqttPublish(WiFi.localIP().toString().c_str(), "log");
}
//MQTT publish
void mqttPublish(const char message[], char mqttChannel[]) {
  char mqttRoute[] = PUBLISHTOPIC;
  strcat(mqttRoute, mqttChannel);  
  client.publish(mqttRoute, message);
  Serial.println("Published MQTT");
}
// MQTT ON MESSAGE
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  pload = "";
  for (int i = 0; i < length; i++) {
    pload.concat((char)payload[i]);
  }
  Serial.println("Received MQTT message");
  Serial.println(pload);
  Serial.println(topic);
  mqttPublish(pload.c_str(), "log");
  unsigned long long x;
  int a = (int)strtol(pload.c_str(), NULL, 0);

  x = a;
 // sscanf(pload.c_str() , "%llx" , &x);

  irsend.sendRC5(x, 12, 2);
  delay(1000);
  Serial.println(int64String(x, HEX));
  x = x +2048;
  Serial.println(int64String(x, HEX));
  irsend.sendRC5(x, 12, 2);
  delay(200);
  Serial.print("Executed RC5: ");
  Serial.println(int64String(x, HEX)); 
 

}

void refreshDisplay(String text) {
  line4 = line3;
  line3 = line2;
  line2 = line1;
  line1 = line0;
  line0 = text;
  display.clear();
  display.setFont(ArialMT_Plain_10);
  
  display.drawStringMaxWidth(0, 2, 500, line3); 
  display.drawStringMaxWidth(0, 14, 500, line2);
  display.drawStringMaxWidth(0, 26, 500, line1);
  display.drawStringMaxWidth(0, 38, 500, line0);
  display.drawStringMaxWidth(0, 50, 500, line0);


  display.display();

}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {

    delay(500);    
  }
 while (!client.connected()) {

    mqttReconnect();
  }
  client.subscribe(SUBTOPIC);
  client.loop();

  
// if code received display it and send some more info via the serial connection
if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_ "\n");
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)

    char prot[]  = "NAN";

    Serial.println("DECODE TYOE");
    Serial.println(results.decode_type);
/* Showing decode type on display is not working right now
    if (results.decode_type == UNKNOWN) {
      char prot[]  = "UNKNOWN";
    } else if (results.decode_type == NEC) {
      char prot[]  = "NEC";
    } else if (results.decode_type == SONY) {
      char prot[]  = "SONY";
    } else if (results.decode_type == 1) {
      char prot[]  = "RC5";
    } else if (results.decode_type == RC5X) {
      char prot[]  = "RC5X";
    } else if (results.decode_type == RC6) {
      char prot[]  = "RC6";
    } else if (results.decode_type == RCMM) {
      char prot[]  = "RCMM";
    } else if (results.decode_type == PANASONIC) {
      char prot[]  = "PANASONIC";
    } else if (results.decode_type == LG) {
      char prot[]  = "LG";
    } else if (results.decode_type == JVC) {
      char prot[]  = "JVC";
    } else if (results.decode_type == AIWA_RC_T501) {
     char prot[]  = "AIWA_RC_T501";
    } else if (results.decode_type == WHYNTER) {
      char prot[]  = "WHYNTER";
    }
*/
    String t = int64String(results.value, HEX);
    const char* val = t.c_str();
    strcat (prot, " 0x");
    strcat(prot, val);
    refreshDisplay(prot);
  }

}
