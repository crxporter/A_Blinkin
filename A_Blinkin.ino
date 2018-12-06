#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
byte myHue = 60;
byte mySat = 70;
byte myVal = 160;

// ********* CHANGE EVERYTHING HERE *********
#define DATA_PIN 2
#define NUM_LEDS 150
const char *MYHOSTNAME = "KitchenESP02";
const char *ssid = "spaceship";
const char *password = "thepassword";
const char *mqtt_server = "192.168.0.124";
const char *mqtt_user = "openhabian";
const char *mqtt_pass = "ohmqtt";
const char *mqtt_client_name = MYHOSTNAME;
// ************** DONE CHANGING *************

ESP8266WebServer server(80);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
CRGB leds[NUM_LEDS];
const int mqtt_port = 1883;
String currentFunction = "None";
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(MYHOSTNAME);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    MDNS.begin(MYHOSTNAME);
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        WiFiUDP::stopAll();
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
        } else {
        }
      }
      yield();
    });
    server.begin();
    MDNS.addService("http", "tcp", 80);
  }
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  ArduinoOTA.setHostname(MYHOSTNAME);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
  });
  ArduinoOTA.begin();
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(1000);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  if (currentFunction == "mychsv") fill_solid(leds, NUM_LEDS, CHSV(myHue, mySat, myVal));
  if (currentFunction == "MakeRed") MakeRed();
  if (currentFunction == "MakeOrange") MakeOrange();
  if (currentFunction == "CenterOut") CenterOut();
  if (currentFunction == "MakePurple") MakePurple();
  if (currentFunction == "PurpleYellow") PurpleYellow();
  if (currentFunction == "RGBChase") RGBChase();
  if (currentFunction == "ColorSweep") ColorSweep();
  if (currentFunction == "BoringSweep") BoringSweep();
  if (currentFunction == "ON") CenterOut();
  if (currentFunction == "CandyCane") CandyCane();
  if (currentFunction == "OFF") fadeToBlackBy (leds, NUM_LEDS, 10);
  if (currentFunction == "Reset") ESP.restart();
  ArduinoOTA.handle();
  server.handleClient();
  FastLED.show();
  delay(10);
}

void reconnect() {
  int retries = 0;
  while (!client.connected()) {
    if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
      client.publish("kitchenws/status", "Ready");
      client.subscribe("kitchenws/function");
      client.subscribe("kitchenws/effect");
      client.subscribe("kitchenws/hue");
      client.subscribe("kitchenws/sat");
      client.subscribe("kitchenws/val");
      break;
    }
    retries++;
    delay(5000);
    if (retries == 150) ESP.restart();
  }
}

void callback(char* topic, byte * payload, unsigned int length) {
  String newTopic = topic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  int intPayload = newPayload.toInt();
  if (newTopic == "kitchenws/function") currentFunction = newPayload;
  if (newTopic == "kitchenws/effect") {
    if (newPayload == "Lightning") Lightning();
  }
  if (newTopic == "kitchenws/hue") myHue = intPayload;
  if (newTopic == "kitchenws/sat") mySat = intPayload;
  if (newTopic == "kitchenws/val") myVal = intPayload;
}

void MakeRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(250);
    client.loop();
    if (currentFunction != "MakeRed") break;
  }
  if (currentFunction == "MakeRed") currentFunction = "None";
}

void MakeOrange() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::OrangeRed;
    FastLED.show();
    delay(250);
    client.loop();
    if (currentFunction != "MakeOrange") break;
  }
  if (currentFunction == "MakeOrange") currentFunction = "None";
}

void CenterOut() {
  currentFunction = "CenterOut";
  for (int i = 0; i < NUM_LEDS - 68; i++) {
    if ( i <= 68) leds[68 - i] = CHSV(myHue, mySat, myVal);
    leds[68 + i] = CHSV(myHue, mySat, myVal);
    FastLED.show();
    delay(50);
    client.loop();
    if (currentFunction != "CenterOut") break;
  }
  if (currentFunction == "CenterOut") currentFunction = "mychsv";
}
void MakePurple() {
  int loopNumber = 0;
  while (currentFunction == "MakePurple") {
    leds[loopNumber] = CRGB::Purple;
    FastLED.show();
    leds[loopNumber] = CRGB::Black;
    delay(20);
    loopNumber ++;
    client.loop();
    if (loopNumber > NUM_LEDS) loopNumber = 0;
  }
}
void PurpleYellow() {
  int loopNumber = 0;
  int gap = 20;
  while (currentFunction == "PurpleYellow") {
    if (loopNumber < NUM_LEDS) leds[loopNumber] = CRGB::Purple;
    if (loopNumber - gap >= 0) leds[loopNumber - gap] = CRGB::Yellow;
    FastLED.show();
    if (loopNumber <  NUM_LEDS) leds[loopNumber] = CRGB::Black;
    if (loopNumber - gap >= 0) leds[loopNumber - gap] = CRGB::Black;
    delay(20);
    loopNumber ++;
    client.loop();
    if (loopNumber > NUM_LEDS + gap) loopNumber = 0;
  }
}
void RGBChase() {
  int LoopNumber = 0;
  int gap1 = 10;
  int gap2 = 20;
  while (currentFunction == "RGBChase") {
    if (LoopNumber < NUM_LEDS) leds[LoopNumber] = CRGB::Red;
    if (LoopNumber - gap1 < NUM_LEDS && LoopNumber - gap1 >= 0) leds[LoopNumber - gap1] = CRGB::Green;
    if (LoopNumber - gap2 >= 0) leds[LoopNumber - gap2] = CRGB::Blue;
    FastLED.show();
    if (LoopNumber < NUM_LEDS) leds[LoopNumber] = CRGB::Black;
    if (LoopNumber - gap1 < NUM_LEDS && LoopNumber - gap1 >= 0) leds[LoopNumber - gap1] = CRGB::Black;
    if (LoopNumber - gap2 >= 0) leds[LoopNumber - gap2] = CRGB::Black;
    delay(20);
    LoopNumber ++;
    client.loop();
    if (LoopNumber > NUM_LEDS + gap2) LoopNumber = 0;
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
      if (currentFunction != "ColorSweep") break;
      if (hue > 255) hue -= 255;
    }
    for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
      leds[i] = CHSV(hue++, 255, 255);
      FastLED.show();
      fadeToBlackBy( leds, NUM_LEDS, 10);
      delay(10);
      client.loop();
      if (currentFunction != "ColorSweep") break;
      if (hue > 255) hue -= 255;
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
      if (currentFunction != "BoringSweep") break;
    }
    hue += 20;
    if (hue > 255) hue -= 255;
    for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
      leds[i] = CHSV(hue, 255, 255);
      FastLED.show();
      fadeToBlackBy( leds, NUM_LEDS, 10);
      delay(10);
      client.loop();
      if (currentFunction != "BoringSweep") break;
    }
    hue += 20;
    if (hue > 255) hue -= 255;
  }
}
void Lightning() {
  // ***Backup previous LED state***
  CRGB backupLeds[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) backupLeds[i] = leds[i];
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(250);
  // ***Define lightning settings***
  uint8_t frequency = 75;
  uint8_t flashes = 10;
  int dimmer = 1;
  uint8_t ledstart = random8(NUM_LEDS - 50);
  uint8_t ledlen = random8(50, NUM_LEDS - ledstart);
  // *** Run lightning loop, will run between 3 and flashes times***
  for (int flashCounter = 0; flashCounter < random8(3, flashes); flashCounter++) {
    if (flashCounter == 0) dimmer = 5;
    else dimmer = random8(1, 3);
    fill_solid(leds + ledstart, ledlen, CHSV(0, 0, 255 / dimmer));
    FastLED.show();
    delay(random8(4, 10));
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    if (flashCounter == 0) delay (250);
    delay(50 + random8(100));
  }
  // ***Restore previous LED state***
  delay(250);
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = backupLeds[i];
  FastLED.show();
}
void CandyCane() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  for (int i = 0; i < NUM_LEDS; i += 5) leds[i] = CRGB::Red;
  for (int i = 1; i < NUM_LEDS; i += 5) leds[i] = CRGB::White;
  for (int i = 2; i < NUM_LEDS; i += 5) leds[i] = CRGB::Green;
  for (int i = 3; i < NUM_LEDS; i += 5) leds[i] = CRGB::White;
  FastLED.show();
  CRGB last;
  while (currentFunction == "CandyCane") {
    last = leds[0];
    for (int i = 0; i < NUM_LEDS - 1; i++) leds[i] = leds[i + 1];
    leds[NUM_LEDS - 1] = last;
    FastLED.show();
    delay(250);
    client.loop();
  }
}
void TwinkleCane() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  for (int i = 0; i < NUM_LEDS; i += 5) leds[i] = CRGB::Red;
  for (int i = 1; i < NUM_LEDS; i += 5) leds[i] = CRGB::Red;
  for (int i = 2; i < NUM_LEDS; i += 5) leds[i] = CRGB::Green;
  for (int i = 3; i < NUM_LEDS; i += 5) leds[i] = CRGB::Green;
  FastLED.show();
  CRGB last;
  int std;
  int stdRisk=80;
  while (currentFunction == "CandyCane") {
    EVERY_N_MILLISECONDS(250) {
      last = leds[0];
      for (int i = 0; i < NUM_LEDS - 1; i++) leds[i] = leds[i + 1];
      leds[NUM_LEDS - 1] = last;
      client.loop();
    }
    if (random8() < stdRisk) {
      std = random16(NUM_LEDS)-1;
      last = leds[std];
      leds[std] = CRGB::White;
      FastLED.show();
      leds[std] = last;
    } else FastLED.show();
    delay(20);
  }
}
