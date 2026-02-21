#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/******** WIFI ********/
const char* ssid = "simran";
const char* password = "12345678";

/******** TELEGRAM ********/
#define BOT_TOKEN "7981008968:AAFRm144vcIJnjn4NyMUpwz8x94oxwOWfjI"
#define CHAT_ID "8318395880"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

/******** GPS ********/
TinyGPSPlus gps;
SoftwareSerial gpsSerial(D5, D6); // RX, TX

/******** OLED ********/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/******** PUSH BUTTON ********/
#define BUTTON_PIN D7   // 🔴 Changed from D3 to D7

bool buttonPressed = false;
unsigned long lastPressTime = 0;

void setup() {

  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  gpsSerial.begin(9600);

  Wire.begin(D2, D1);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  showStartupScreen();

  connectWiFi();

  client.setInsecure();   // Required for Telegram
}

/* ------------------ LOOP ------------------ */

void loop() {

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastPressTime > 1000) {
    lastPressTime = millis();
    sendSOS();
  }
}

/* ------------------ FUNCTIONS ------------------ */

void showStartupScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(10, 20);
  display.println("Women Safety SOS");

  display.setCursor(25, 35);
  display.println("System Starting...");
  display.display();
  delay(2000);
}

void connectWiFi() {

  display.clearDisplay();
  display.setCursor(0, 20);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeout++;

    if (timeout > 40) {   // 20 sec timeout
      display.clearDisplay();
      display.setCursor(10, 25);
      display.println("WiFi Failed!");
      display.display();
      Serial.println("\nWiFi Failed");
      return;
    }
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(10, 25);
  display.println("WiFi Connected!");
  display.display();
  delay(2000);
}

void sendSOS() {

  display.clearDisplay();
  display.setCursor(15, 25);
  display.println("Getting GPS...");
  display.display();

  unsigned long start = millis();

  while (!gps.location.isValid() && millis() - start < 10000) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
  }

  if (!gps.location.isValid()) {
    display.clearDisplay();
    display.setCursor(10, 25);
    display.println("GPS Not Fixed!");
    display.display();
    delay(2000);
    return;
  }

  float lat = gps.location.lat();
  float lng = gps.location.lng();

  display.clearDisplay();
  display.setCursor(10, 20);
  display.println("Sending SOS...");
  display.display();

  String message = "🚨 WOMEN SAFETY SOS ALERT!\n\n";
  message += "Emergency Help Needed!\n\n";
  message += "Location:\n";
  message += "https://maps.google.com/?q=";
  message += String(lat, 6);
  message += ",";
  message += String(lng, 6);

  bool sent = bot.sendMessage(CHAT_ID, message, "");

  display.clearDisplay();
  display.setCursor(10, 25);

  if (sent) {
    display.println("Location Sent!");
    Serial.println("Message Sent");
  } else {
    display.println("Send Failed!");
    Serial.println("Telegram Failed");
  }

  display.display();
  delay(3000);

  display.clearDisplay();
  display.setCursor(20, 25);
  display.println("System Ready");
  display.display();
}