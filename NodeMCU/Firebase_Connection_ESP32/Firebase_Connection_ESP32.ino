// ESP32 version of Firebase_Connection.ino (converted from NodeMCU/ESP8266)
//
// Libraries required (install via Library Manager):
//  - "Firebase ESP32 Client" by mobizt  (a.k.a. Firebase-ESP-Client)
//    *OR* the legacy "FirebaseESP32" library. This sketch uses the legacy API for minimal diffs.
//  - "ESP32" board package by Espressif Systems
//
// If you cannot find "FirebaseESP32", install "Firebase ESP32 Client" and follow the notes below.
//
// ======================= IMPORTANT =======================
// Replace FIREBASE_HOST with your RTDB host, e.g. "your-project-id.firebaseio.com"
// Replace FIREBASE_AUTH with your RTDB secret (Database -> Rules -> Legacy Token) if using legacy auth.
// =========================================================

#include <WiFi.h>
// Use one of the following Firebase libraries:
// Legacy (similar API to your original):
#include "FirebaseESP32.h"
// Modern library (alternative):
// #include <Firebase_ESP_Client.h>  // then follow the "NOTE: Modern library" section below

// ---- User Config ----
//#define FIREBASE_HOST "your-project-id.firebaseio.com"  // e.g. "my-app-default-rtdb.firebaseio.com"
//#define FIREBASE_AUTH "YOUR_DATABASE_SECRET"
//#define WIFI_SSID     "YOUR_WIFI_SSID"
//#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

//----ivp info---
#define FIREBASE_HOST "nodemcu-6a1b8-wheelchair-default-rtdb.firebaseio.com"  //Change to your Firebase RTDB project ID e.g. Your_Project_ID.firebaseio.com
#define FIREBASE_AUTH "fCcgpXcBfCDhSk8DAZDjj7iHngymPZ5OBXJTkD0h" //Change to your Firebase RTDB secret password
#define WIFI_SSID "phoque6's Galaxy S10+"
#define WIFI_PASSWORD "rtui3148"

// Path to listen for commands
String path = "/Position";

// Map former NodeMCU pins D1 (GPIO5) and D2 (GPIO4) to ESP32 GPIOs
// You can change these to any safe GPIOs, e.g., 4 and 5 are OK on most ESP32 dev boards.
const int PIN_OUT1 = 5;  // was D1 on NodeMCU
const int PIN_OUT2 = 4;  // was D2 on NodeMCU

// Firebase data object
FirebaseData fbdo;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_OUT1, OUTPUT);
  pinMode(PIN_OUT2, OUTPUT);
  digitalWrite(PIN_OUT1, LOW);
  digitalWrite(PIN_OUT2, LOW);

  connectWiFi();

  // ---- Legacy FirebaseESP32 style ----
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(fbdo, path)) {
    Serial.println("Could not begin stream");
    Serial.println("Reason: " + fbdo.errorReason());
  } else {
    Serial.println("Stream started on: " + path);
  }
}

void loop() {
  // Keep the stream alive and read new data
  if (!Firebase.readStream(fbdo)) {
    Serial.println("Stream read failed");
    Serial.println("Reason: " + fbdo.errorReason());
    delay(500);
    return;
  }

  if (fbdo.streamTimeout()) {
    Serial.println("Stream timeout, resuming...");
  }

  if (fbdo.streamAvailable()) {
    if (fbdo.dataType() == "string") {
      String str = fbdo.stringData();
      str.trim();
      Serial.print("Command: ");
      Serial.println(str);

      if (str.equalsIgnoreCase("Left")) {
        digitalWrite(PIN_OUT1, HIGH);
        digitalWrite(PIN_OUT2, LOW);
      } else if (str.equalsIgnoreCase("Right")) {
        digitalWrite(PIN_OUT1, LOW);
        digitalWrite(PIN_OUT2, HIGH);
      } else if (str.equalsIgnoreCase("Center")) {
        digitalWrite(PIN_OUT1, HIGH);
        digitalWrite(PIN_OUT2, HIGH);
      } else if (str.equalsIgnoreCase("Stop")) {
        digitalWrite(PIN_OUT1, LOW);
        digitalWrite(PIN_OUT2, LOW);
      } else {
        Serial.println("Unknown command");
      }
    } else {
      Serial.print("Non-string data at ");
      Serial.print(fbdo.dataPath());
      Serial.print(" type: ");
      Serial.println(fbdo.dataType());
    }
  }

  delay(10);
}

/* ================== NOTE: Modern library (Firebase_ESP_Client) ==================
If you prefer/need the modern library, use this template instead of the legacy includes:

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/RTDBHelper.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_OUT1, OUTPUT);
  pinMode(PIN_OUT2, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }

  config.database_url = "https://your-project-id.firebaseio.com";
  config.signer.tokens.legacy_token = FIREBASE_AUTH; // if using database secret

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo, path.c_str())) {
    Serial.println(fbdo.errorReason());
  }
}

void loop() {
  if (!Firebase.RTDB.readStream(&fbdo)) {
    Serial.println(fbdo.errorReason());
    return;
  }
  if (fbdo.streamAvailable()) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string) {
      String str = fbdo.to<const char*>();
      // handle commands as above...
    }
  }
}
================================================================================== */
