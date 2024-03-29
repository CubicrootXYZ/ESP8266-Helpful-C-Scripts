#include <EventManager.h> //event lib
#include <ESP8266WiFi.h>  //wifi lib
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <time.h> // https://playground.arduino.cc/Code/Time/
#include <Timezone.h> // https://github.com/JChristensen/Timezone
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>

#define wifi_ssid "SSID"
#define wifi_password "PASSWORD"
#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 128
#define HASS_TOKEN "ADD HERE"
#define HASS_URL "https://yourdomain.tld/api"

// ICONS

const unsigned char icon_cloudy [] PROGMEM = {
  0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xfc, 0x3f, 0xf0, 0xf0, 0x0f, 0xf0, 0xf0, 
  0x00, 0xf0, 0xe0, 0x00, 0x70, 0xe0, 0x00, 0x70, 0xc0, 0x00, 0x70, 0x80, 0x00, 0x30, 0x00, 0x00, 
  0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 
  0xc0, 0x00, 0x30, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0
};

const unsigned char icon_rainy [] PROGMEM = {
  0xf8, 0x7f, 0xf0, 0xf0, 0x33, 0xf0, 0xe0, 0x00, 0xf0, 0xe0, 0x00, 0xf0, 0xe0, 0x00, 0xf0, 0xc0, 
  0x00, 0x30, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0x00, 0x10, 0xf0, 0x00, 0xf0, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xef, 0x9f, 0x70, 
  0xc7, 0x9e, 0x30, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xe7, 0x9e, 0x70
};

const unsigned char icon_exceptional [] PROGMEM = {
  0xff, 0xff, 0xc0, 0xff, 0xce, 0x00, 0xff, 0x80, 0x10, 0xfe, 0x00, 0x10, 0xf8, 0x00, 0x10, 0xf0, 
  0x00, 0x10, 0xe0, 0x00, 0x30, 0xc0, 0x00, 0x30, 0x87, 0xc0, 0x10, 0x8f, 0xe0, 0x10, 0x19, 0xf0, 
  0x30, 0x19, 0xf0, 0x70, 0x1f, 0xf0, 0x70, 0x1f, 0xf0, 0xf0, 0x1f, 0x70, 0xf0, 0x8f, 0xe1, 0xf0, 
  0x87, 0xc3, 0xf0, 0xc0, 0x07, 0xf0, 0xe0, 0x0f, 0xf0, 0xf8, 0x3f, 0xf0
};

const unsigned char icon_windy [] PROGMEM = {
  0xff, 0xff, 0xf0, 0xff, 0xf1, 0xf0, 0xff, 0xe0, 0x70, 0xff, 0xc0, 0x70, 0xff, 0xc6, 0x70, 0xff, 
  0xfe, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x70, 0x00, 0x01, 0xf0, 0xff, 0xff, 0xf0, 0x00, 0xc0, 
  0x30, 0x00, 0x60, 0x10, 0x80, 0x20, 0x00, 0xfe, 0x3f, 0x80, 0xe6, 0x3f, 0x80, 0xe0, 0x33, 0x80, 
  0xf0, 0x70, 0x10, 0xff, 0xf8, 0x10, 0xff, 0xfc, 0x70, 0xff, 0xff, 0xf0
};

const unsigned char icon_sunny [] PROGMEM = {
  0xff, 0x9f, 0xf0, 0xff, 0x9f, 0xf0, 0xff, 0x0f, 0xf0, 0xe3, 0x0c, 0x70, 0xe0, 0x00, 0x70, 0xe0, 
  0xf0, 0x70, 0xf3, 0x9c, 0xf0, 0xf2, 0x04, 0xf0, 0xc6, 0x06, 0x30, 0x04, 0x02, 0x00, 0x04, 0x02, 
  0x00, 0xc6, 0x06, 0x30, 0xf2, 0x04, 0xf0, 0xf3, 0x9c, 0xf0, 0xe0, 0xf0, 0x70, 0xe0, 0x00, 0x70, 
  0xe3, 0x0c, 0x70, 0xff, 0x0f, 0xf0, 0xff, 0x9f, 0xf0, 0xff, 0x9f, 0xf0
};

const unsigned char icon_snowy [] PROGMEM = {
  0xff, 0x9f, 0xf0, 0xfe, 0x97, 0xf0, 0xfc, 0x03, 0xf0, 0xf6, 0x06, 0xf0, 0xd3, 0x0c, 0xb0, 0x83, 
  0x9c, 0x10, 0xc3, 0x9c, 0x30, 0xc0, 0x90, 0x30, 0x80, 0x00, 0x10, 0xfe, 0x07, 0xf0, 0xfe, 0x07, 
  0xf0, 0x80, 0x00, 0x10, 0xc0, 0x90, 0x30, 0xc3, 0x9c, 0x30, 0x83, 0x9c, 0x10, 0xd3, 0x0c, 0xb0, 
  0xf6, 0x06, 0xf0, 0xfe, 0x07, 0xf0, 0xfe, 0x97, 0xf0, 0xff, 0x9f, 0xf0
};

const unsigned char icon_pouring [] PROGMEM = {
  0xf8, 0x7f, 0xf0, 0xf0, 0x33, 0xf0, 0xe0, 0x00, 0xf0, 0xe0, 0x00, 0xf0, 0xe0, 0x00, 0xf0, 0xc0, 
  0x00, 0x30, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0x00, 0x10, 0xf0, 0x00, 0xf0, 0xff, 0xff, 0xf0, 0xff, 0xdd, 0xf0, 0xcd, 0xd9, 0x90, 
  0xd9, 0x9b, 0xb0, 0xbb, 0x33, 0x70, 0x33, 0x76, 0x70, 0x77, 0x66, 0xf0
};

const unsigned char icon_partlycloudy [] PROGMEM = {
  0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xfc, 0xff, 0xf0, 0xfc, 0xff, 0xf0, 0xc8, 0x4f, 0xf0, 0xc0, 
  0x0f, 0xf0, 0xe4, 0x9f, 0xf0, 0xc8, 0x7f, 0xf0, 0x00, 0x47, 0xf0, 0x00, 0x83, 0xf0, 0xc9, 0x00, 
  0x70, 0xe5, 0x00, 0x30, 0xc6, 0x00, 0x10, 0xcc, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00, 
  0xfc, 0x00, 0x00, 0xfe, 0x00, 0x10, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0
};

const unsigned char icon_lightning [] PROGMEM = {
  0xff, 0xcf, 0xf0, 0xff, 0xc7, 0xf0, 0xff, 0xc3, 0xf0, 0xff, 0xc3, 0xf0, 0xff, 0x83, 0xf0, 0xf8, 
  0x01, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xf0, 0x00, 0xf0, 0xc1, 0xf8, 0x30, 0xc1, 0x08, 
  0x30, 0x81, 0x18, 0x10, 0x81, 0x1e, 0x10, 0xc3, 0x02, 0x30, 0xe3, 0x06, 0x70, 0xff, 0x07, 0xf0, 
  0xff, 0xcf, 0xf0, 0xff, 0xcf, 0xf0, 0xff, 0xdf, 0xf0, 0xff, 0xbf, 0xf0
};

const unsigned char icon_hail [] PROGMEM = {
  0xf8, 0x7f, 0xf0, 0xf0, 0x33, 0xf0, 0xe0, 0x00, 0xf0, 0xe0, 0x00, 0xf0, 0xe0, 0x00, 0xf0, 0xc0, 
  0x00, 0x30, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 
  0x00, 0x83, 0x9c, 0x10, 0xf6, 0x06, 0xf0, 0xfc, 0x03, 0xf0, 0x1c, 0x03, 0x80, 0x0c, 0x03, 0x00, 
  0x0c, 0x03, 0x00, 0xfc, 0x03, 0xf0, 0xfe, 0x07, 0xf0, 0xff, 0x9f, 0xf0
};

const unsigned char icon_fog [] PROGMEM = {
  0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xc1, 0x87, 0xf0, 0x80, 0x03, 0xf0, 0x00, 0x00, 0x30, 0x00, 
  0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0xc0, 0x00, 
  0x30, 0xff, 0xc3, 0xf0, 0xff, 0xff, 0xf0, 0xe0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xff, 0xff, 0xf0, 
  0x86, 0x00, 0x30, 0x04, 0x00, 0x30, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0
};

const unsigned char icon_clear_night [] PROGMEM = {
  0xff, 0x07, 0xf0, 0xfc, 0x0f, 0xf0, 0xf8, 0x1f, 0xf0, 0xf0, 0x3f, 0xf0, 0xe0, 0x3f, 0xf0, 0xc0, 
  0x7f, 0xf0, 0xc0, 0x7f, 0xf0, 0x80, 0x7f, 0xf0, 0x80, 0x7f, 0xf0, 0x80, 0x7f, 0xf0, 0x80, 0x3f, 
  0xf0, 0x80, 0x3f, 0xf0, 0x80, 0x1f, 0xf0, 0xc0, 0x0f, 0xf0, 0xc0, 0x07, 0xf0, 0xe0, 0x01, 0xf0, 
  0xf0, 0x00, 0x10, 0xf0, 0x00, 0x30, 0xfc, 0x00, 0xf0, 0xff, 0x03, 0xf0
};

WiFiClient espClient;

unsigned long int globalRuntime = 0;
unsigned long int milliseconds = 0;
unsigned long displayTime = 0;
unsigned long clockTime = 0;
unsigned long clockTimeNTP = 60001;
unsigned long metadataTime = 60001;
double temperature = -99.0;
double windspeed = -1.0;
char weather[15];

// Display stuff
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
EventManager gEM;

// Time stuff
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP); // Updates all 60 seconds

// Timezone
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

void setup() {
  // Serial
  Serial.begin(9600);
  Serial.setTimeout(2000);
  while(!Serial) { }
  // Start I2C
  Wire.begin();
  // Display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(2);
  display.display();
  // WIFI
  setup_wifi();
  Serial.println(WiFi.localIP().toString());
  // Clear the display buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
  // NTP client
  timeClient.begin();
  // Event Handler
  gEM.addListener( EventManager::kEventUser0, refreshDisplay);
  gEM.addListener( EventManager::kEventUser2, refreshTimeNTP);
  gEM.addListener( EventManager::kEventUser3, updateMetadata);
}

void setup_wifi() {
  delay(5);
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

void refreshDisplay(int event, int param) {
  //Serial.println("Refreshing Display");
  display.clearDisplay();
  display.setRotation(2);

  //Display time
  char seconds_str[4];
  sprintf(seconds_str, "%02d", second());
  char minutes_str[4];
  sprintf(minutes_str, "%02d", minute());
  char hours_str[4];
  sprintf(hours_str, "%02d", hour());

  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.drawRect(0,0,128,25, SSD1306_WHITE);
  // Blinking dots
  display.setTextSize(3);
  if (millis()%1000 >= 500) {
    display.setCursor(33,1);
    display.println(":");
    display.setCursor(78,1);
    display.println(":");
  } else {
    display.setCursor(33,1);
    display.println(" ");
    display.setCursor(78,1);
    display.println(" ");
  }
  display.setCursor(1,1);
  display.println(hours_str);
  display.setCursor(47,1);
  display.println(minutes_str);
  display.setCursor(92,1);
  display.println(seconds_str);

  display.setTextColor(SSD1306_WHITE);

  // Icon
  yield();
  if (strcmp(weather, "cloudy") == 0) {
    display.drawBitmap(76, 26, icon_cloudy, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "rainy") == 0 || strcmp(weather, "snowy-rainy") == 0) {
    display.drawBitmap(76, 26, icon_rainy, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "clear-night") == 0) {
    display.drawBitmap(76, 26, icon_clear_night, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "fog") == 0) {
    display.drawBitmap(76, 26, icon_fog, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "hail") == 0) {
    display.drawBitmap(76, 26, icon_hail, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "lightning") == 0 || strcmp(weather, "lightning-rainy") == 0) {
    display.drawBitmap(76, 26, icon_lightning, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "partlycloudy") == 0) {
    display.drawBitmap(76, 26, icon_partlycloudy, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "pouring") == 0) {
    display.drawBitmap(76, 26, icon_pouring, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "snowy") == 0) {
    display.drawBitmap(76, 26, icon_snowy, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "sunny") == 0) {
    display.drawBitmap(76, 26, icon_sunny, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "windy") == 0 || strcmp(weather, "windy-variant") == 0) {
    display.drawBitmap(76, 26, icon_windy, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  } else if (strcmp(weather, "exceptional") == 0) {
    display.drawBitmap(76, 26, icon_exceptional, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  }

  if (windspeed > 17.5) {
    display.drawBitmap(106, 26, icon_windy, 20, 20, SSD1306_BLACK, SSD1306_WHITE);
  }

  // Display Metadata
  display.setCursor(0,30);
  display.setTextSize(2);
  char temp_str[4];
  sprintf(temp_str, "%-.1fC", temperature);
  display.println(temp_str);

  // Display Date
  char date_str[10];
  sprintf(date_str, "%02d.%02d.%04d", day(), month(), year());
  yield();

  display.setTextSize(2);
  display.setCursor(6,50);
  display.println(date_str);

  // Display it
  display.setRotation(2);
  display.display();

  displayTime = millis();
}

void refreshTimeNTP(int event, int param) {
  if (timeClient.update()){
     unsigned long epoch = timeClient.getEpochTime();
     yield();
     setTime(CE.toLocal(epoch));
  }

  clockTimeNTP = millis();
}

void updateMetadata(int event, int param) {
  String url = HASS_URL + String("/states/weather.home");
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  yield();

  http.begin(client, url.c_str());

  http.setAuthorization("");
  http.addHeader("Authorization",String("Bearer ") + HASS_TOKEN);
  int responseCode = http.GET();
  yield();

  if (responseCode == 200) {
    String payload = http.getString();
    // Parse JSON
    JSONVar myObject = JSON.parse(payload);
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }
    yield();

    temperature = double(myObject["attributes"]["temperature"]);
    windspeed = double(myObject["attributes"]["wind_speed"]);
    strcpy( weather, (const char*)myObject["state"]);
  
  } else {
    Serial.print("Hass request failed with: ");
    Serial.println(responseCode);
    String payload = http.getString();
    Serial.print(payload);
  }
  yield();
  http.end();

  metadataTime = millis();
}

// LOOP
void loop() {
  if (millis() > 43200000) {
    ESP.restart();
  }

  while (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
    delay(500);    
  }

  // Handle any events that are in the queue
  gEM.processEvent();

  //refreshDisplay
  if ( ( millis() - displayTime ) > 100 )
  {
      gEM.queueEvent( EventManager::kEventUser0, 0);
  }
  //refreshTimeNTP
  if ( ( millis() - clockTimeNTP ) > 60000 )
  {
      gEM.queueEvent( EventManager::kEventUser2, 0);
  }

   if ( ( millis() - metadataTime ) > 60000 )
  {
      gEM.queueEvent( EventManager::kEventUser3, 0);
  }
}
