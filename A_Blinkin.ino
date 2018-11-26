// ************************** VARIABLES ***********************
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SimpleTimer.h>
#include <FastLED.h>
#define DATA_PIN 2
#define NUM_LEDS 150
CRGB leds[NUM_LEDS];
byte myHue = 60;
byte mySat = 70;
byte myVal = 160;
const char *MYHOSTNAME = "KitchenESP02";

// ************************* WIFI SETUP ***********************
const char *ssid = "spaceship";
const char *password = "thepassword";
const char *mqtt_server = "192.168.0.124";
const int mqtt_port = 1883;
const char *mqtt_user = "openhabian";
const char *mqtt_pass = "ohmqtt";

const char *mqtt_client_name = MYHOSTNAME;
String currentFunction = "Manual";
WiFiClient espClient;
PubSubClient client(espClient);

SimpleTimer timer;

void setup() {
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(MYHOSTNAME);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  ArduinoOTA.setHostname(MYHOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
  });
  ArduinoOTA.begin();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  fill_solid(leds, NUM_LEDS, CRGB::Purple);
  FastLED.show();
  delay(1000);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (currentFunction == "OFF") {
    fadeToBlackBy( leds, NUM_LEDS, 10);
  }
  if (currentFunction != "Manual") {
    if (currentFunction == "MakeRed") {
      MakeRed();
    }
    else if (currentFunction == "MakeOrange") {
      MakeOrange();
    }
    else if (currentFunction == "CenterOut") {
      CenterOut();
    }
    else if (currentFunction == "MakePurple") {
      MakePurple();
    }
    else if (currentFunction == "ColorSweep") {
      ColorSweep();
    }
    else if (currentFunction == "BoringSweep") {
      BoringSweep();
    }
    else if (currentFunction == "ON") {
      CenterOut();
      //      fill_solid(leds, NUM_LEDS, CHSV(60, 70, 160));
    }
    else if (currentFunction == "mychsv") {
      fill_solid(leds, NUM_LEDS, CHSV(myHue, mySat, myVal));
    }
  }
  timer.run();
  ArduinoOTA.handle();
  FastLED.show();
  delay(10);
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  int retries = 0;
  while (!client.connected()) {
    if (retries < 150)  {
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
        client.publish("kitchenws/status", "Ready");
        client.subscribe("kitchenws/function");
        client.subscribe("kitchenws/hue");
        client.subscribe("kitchenws/sat");
        client.subscribe("kitchenws/val");
      }
      else {
        retries++;
        delay(5000);
      }
    }
    if (retries > 1000) {
      ESP.restart();
    }
  }
}

void callback(char* topic, byte * payload, unsigned int length) {
  String newTopic = topic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  int intPayload = newPayload.toInt();
  if (newTopic == "kitchenws/function") {
    currentFunction = newPayload;
  }
  if (newTopic == "kitchenws/hue") {
    myHue = intPayload;
  }
  if (newTopic == "kitchenws/sat") {
    mySat = intPayload;
  }
  if (newTopic == "kitchenws/val") {
    myVal = intPayload;
  }
}

void MakeRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(250);
    client.loop();
    if (currentFunction != "MakeRed") break;
  }
  if (currentFunction == "MakeRed") {
    currentFunction = "Manual";
  }
}

void MakeOrange() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::OrangeRed;
    FastLED.show();
    delay(250);
    client.loop();

  }
  if (currentFunction == "MakeOrange") {
    currentFunction = "Manual";
  }
}

void CenterOut() {
  for (int i = 0; i < NUM_LEDS - 68; i++) {
    if ( i <= 68) {
      leds[68 - i] = CHSV(myHue, mySat, myVal);
    }
    leds[68 + i] = CHSV(myHue, mySat, myVal);
    FastLED.show();
    delay(50);
    client.loop();
    if ( i == NUM_LEDS - 69) {
      currentFunction = "Manual";
      client.publish("kitchenws/status", "manual mode");
      client.publish("kitchenws/status", "Ready");
    }
  }
}

void MakePurple() {
  int purpleLed = 0;
  while (currentFunction == "MakePurple") {
    if (purpleLed) leds[purpleLed] = CRGB::Purple;
    FastLED.show();
    if (purpleLed <  NUM_LEDS) leds[purpleLed] = CRGB::Black;
    delay(20);
    purpleLed ++;
    client.loop();
    if (purpleLed > NUM_LEDS) {
      purpleLed = 0;
    }
  }
}
void ColorSweep() {
  static uint8_t hue = 0;
  while (currentFunction == "ColorSweep") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue++, 255, 255);
      FastLED.show();
      fadeToBlackBy( leds, NUM_LEDS, 10);
      delay(10);
      client.loop();
      if (hue > 255) {
        hue -= 255;
      }
    }
    for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
      leds[i] = CHSV(hue++, 255, 255);
      FastLED.show();
      fadeToBlackBy( leds, NUM_LEDS, 10);
      delay(10);
      client.loop();
      if (hue > 255) {
        hue -= 255;
      }
    }
  }
}
void BoringSweep() {
  static uint8_t hue = 0;
  while (currentFunction == "BoringSweep") {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, 255, 255);
      FastLED.show();
      fadeToBlackBy( leds, NUM_LEDS, 10);
      delay(10);
      client.loop();
    }
    hue += 20;
    for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
      leds[i] = CHSV(hue, 255, 255);
      FastLED.show();
      fadeToBlackBy( leds, NUM_LEDS, 10);
      delay(10);
      client.loop();
    }
    hue += 20;
    if (hue > 255) {
      hue -= 255;
    }
  }
}
void ManualMode() {
  currentFunction == "Manual";
  client.publish("kitchenws/status", "manual mode");

}
