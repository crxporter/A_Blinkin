// ************************** VARIABLES ***********************

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#define DATA_PIN 5
#define NUM_LEDS 150
CRGB leds[NUM_LEDS];

// ************************* WIFI SETUP ***********************
//*
const char* ssid = "CIA Surveillance Van";
const char* password = "N6u7ffnK";
const char* mqtt_server = "192.168.1.203";
const int mqtt_port = 1883;
const char *mqtt_user = "brian";
const char *mqtt_pass = "mqttbrian";
//*/


const char *mqtt_client_name = "BlinkyMcBlinkin";
String currentFunction = "Manual";
WiFiClient espClient;
PubSubClient client(espClient);

// ********* OTHER *********


void setup() {
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  ArduinoOTA.setHostname("aBlinkin");
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

  if (currentFunction != "Manual") {
    if (currentFunction == "MakeRed") {
      MakeRed();
    }

    else if (currentFunction == "MakeOrange") {
      MakeOrange();
    }
    else if (currentFunction == "MakeGreen") {
      MakeGreen();
    }
    else if (currentFunction == "MakePurple") {
      MakePurple();
    }
    else if (currentFunction == "MakePurplechase") {
      MakePurplechase();
    }
    else if (currentFunction == "OFF") {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
    else if (currentFunction == "Restart") {
      ESP.restart();

    }
  }

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
        client.publish("ablinkin/status", "Ready");
        client.subscribe("ablinkin/function");
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

  if (newTopic == "ablinkin/function") {
    currentFunction = newPayload;
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
    if (currentFunction != "MakeOrange") break;

  }
  if (currentFunction == "MakeOrange") {
    currentFunction = "Manual";
  }
}

void MakeGreen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
    FastLED.show();
    delay(250);
    client.loop();
    if (currentFunction != "MakeGreen") break;

  }
  if (currentFunction == "MakeGreen") {
    currentFunction = "Manual";
  }
}

void MakePurple() {
  int purpleLed = 0;
  while (currentFunction == "MakePurple") {
    leds[purpleLed] = CRGB::Purple;
    FastLED.show();
    leds[purpleLed] = CRGB::Black;
    delay(20);
    purpleLed ++;
    client.loop();
    if (purpleLed > NUM_LEDS) {
      purpleLed = 0;
      if (currentFunction != "MakePurple") break;
    }
  }
}
void MakePurplechase() {
  int purpleLed = 0;
  int gap = 20;
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  while (currentFunction == "MakePurplechase") {
     if (purpleLed < NUM_LEDS) leds[purpleLed] = CRGB::Purple;
     if (purpleLed - gap >= 0) leds[purpleLed - gap] = CRGB::Yellow;
     FastLED.show();
     if (purpleLed < NUM_LEDS) leds[purpleLed] = CRGB::Black;
     if (purpleLed - gap >= 0) leds[purpleLed - gap] = CRGB::Black;
     delay(20);
     purpleLed ++;
     client.loop();
     if (purpleLed > NUM_LEDS + gap) purpleLed = 0;
  }
}
