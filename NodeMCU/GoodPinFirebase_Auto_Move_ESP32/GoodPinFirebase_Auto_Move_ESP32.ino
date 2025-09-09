// ESP32 Firebase-controlled L298N Robot
//
// This merges your Auto_Move pinout with Firebase streaming commands.
// Commands expected at RTDB path "/Position": "Forward", "Back", "Left", "Right", "Stop", "Center".
//
// Required:
//  - Boards Manager: "ESP32 by Espressif Systems"
//  - Library Manager: "FirebaseESP32" by mobizt (legacy) 
//      (Alternatively use "Firebase ESP Client" -> see notes at bottom for API differences.)
//
// Edit these:
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


#include <WiFi.h>
#include "FirebaseESP32.h"

// ===== Pinout from 1_Auto_Move.ino =====
#define ENA_PIN 25
#define IN1     27
#define IN2     26
#define IN3     12  // NOTE: GPIO12 is a strapping pin; avoid pulling it HIGH during boot externally.
#define IN4     14
#define ENB_PIN 13

// LEDC PWM channels (0–15 valid on ESP32)
#define ENA_CH 0
#define ENB_CH 1

// PWM parameters
#define PWM_FREQ 5000  // 5 kHz
#define PWM_RES  8     // 8-bit (0–255)

// Default speed
uint8_t CAR_SPEED = 150;

// Firebase objects
FirebaseData fbdo;
String path = "/Position";

// ---- Motion helpers ----
static inline void setSpeed(uint8_t a, uint8_t b) {
  ledcWrite(ENA_CH, a);
  ledcWrite(ENB_CH, b);
}

void Forward(){ 
  setSpeed(CAR_SPEED, CAR_SPEED);
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH);  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Forward");
}

void Back() {
  setSpeed(CAR_SPEED, CAR_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Back");
}

void Right() {
  setSpeed(CAR_SPEED, CAR_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Right");
}

void Left() {
  setSpeed(CAR_SPEED, CAR_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW); 
  Serial.println("Left");
}

void Stop() {
  setSpeed(0, 0);  // coast to stop
  Serial.println("Stop");
}

// Optional: fast brake (hold both inputs same state with EN high)
// Not used by default, but you can call Brake() instead of Stop() if desired.
void Brake() {
  ledcWrite(ENA_CH, 255);
  ledcWrite(ENB_CH, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  Serial.println("Brake");
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi: ");
  Serial.print(WIFI_SSID);
  Serial.print(" ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Motor IO
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // LEDC setup
  ledcSetup(ENA_CH, PWM_FREQ, PWM_RES);
  ledcSetup(ENB_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENA_PIN, ENA_CH);
  ledcAttachPin(ENB_PIN, ENB_CH);

  Stop();

  // WiFi + Firebase
  wifiConnect();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(fbdo, path)) {
    Serial.println("Could not begin stream");
    Serial.println("Reason: " + fbdo.errorReason());
  } else {
    Serial.println("Stream started on " + path);
  }
}

void loop() {
  if (!Firebase.readStream(fbdo)) {
    Serial.print("Stream read failed: ");
    Serial.println(fbdo.errorReason());
    delay(200);
    return;
  }

  if (fbdo.streamTimeout()) {
    Serial.println("Stream timeout, resuming...");
  }

  if (fbdo.streamAvailable()) {
    if (fbdo.dataType() == "string") {
      String cmd = fbdo.stringData();
      cmd.trim();
      cmd.toLowerCase();

      Serial.print("Command: "); Serial.println(cmd);

      if (cmd == "forward")       Forward();
      else if (cmd == "back")     Back();
      else if (cmd == "left")     Left();
      else if (cmd == "right")    Right();
      else if (cmd == "stop")     Stop();
      else if (cmd == "center")   Stop();   // alias for Stop; change to Brake() if you prefer fast brake
      else if (cmd.startsWith("speed:")) {  // e.g., "speed:200"
        int v = cmd.substring(6).toInt();
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        CAR_SPEED = (uint8_t)v;
        Serial.print("Speed set to "); Serial.println(CAR_SPEED);
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

  delay(5);
}

/* ===== If using "Firebase ESP Client" instead =====
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
void setup() {
  // ... WiFi connect ...
  config.database_url = "https://your-project-id-default-rtdb.firebaseio.com";
  config.signer.tokens.legacy_token = FIREBASE_AUTH; // or other auth method
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.beginStream(&fbdo, path.c_str());
}
void loop() {
  if (Firebase.RTDB.readStream(&fbdo)) {
    if (fbdo.streamAvailable() && fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string) {
      String cmd = fbdo.to<const char*>();
      // same command handling as above
    }
  }
}
*/
