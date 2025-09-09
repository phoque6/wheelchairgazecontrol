// Robot_Firebase_ESP32.ino
// ESP32 + L298N motor driver controlled via Firebase Realtime Database (Mobizt v4.4.17 library)
//
// Library Manager:
//   - "Firebase Arduino Client Library for ESP8266 and ESP32" by Mobizt (v4.4.17 or latest)
// Boards Manager:
//   - "ESP32 by Espressif Systems"
//
// RTDB path listened: "/Position"
// Supported commands (string):
//   "forward", "back", "left", "right", "stop", "center"
//   "speed:<0..255>"   e.g., "speed:200"
//
// ---- User settings ----
//Edit these:
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

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String path = "/Position";

// ===== L298N wiring (matches your Auto_Move.ino) =====
#define ENA_PIN 25
#define IN1     27
#define IN2     26
#define IN3     12   // strap pin: safe as OUTPUT after boot
#define IN4     14
#define ENB_PIN 13

// LEDC (ESP32 PWM)
#define ENA_CH 0
#define ENB_CH 1
#define PWM_FREQ 5000
#define PWM_RES  8     // 0..255
uint8_t CAR_SPEED = 150;

// ---------- Motor helpers ----------
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
  setSpeed(0, 0);
  Serial.println("Stop");
}

// Optional: active brake (hold both inputs)
void Brake() {
  ledcWrite(ENA_CH, 255);
  ledcWrite(ENB_CH, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
  Serial.println("Brake");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // LEDC PWM config
  ledcSetup(ENA_CH, PWM_FREQ, PWM_RES);
  ledcSetup(ENB_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENA_PIN, ENA_CH);
  ledcAttachPin(ENB_PIN, ENB_CH);
  Stop();

  connectWiFi();

  // Firebase config
  config.database_url = DATABASE_URL;

  // --- Choose ONE auth method ---
  // Legacy DB secret (simplest for quick tests):
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  // (Alternative) API key + email/password or anonymous sign-in:
  // config.api_key = "YOUR_WEB_API_KEY";
  // auth.user.email = "email@example.com";
  // auth.user.password = "password";
  // or: Firebase.signUp(&config, &auth, "", ""); // anonymous

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo, path.c_str())) {
    Serial.println("Stream start failed: " + fbdo.errorReason());
  } else {
    Serial.println("Streaming on " + path);
  }
}

void loop() {
  if (!Firebase.RTDB.readStream(&fbdo)) {
    Serial.println("Stream read failed: " + fbdo.errorReason());
    delay(200);
    return;
  }

  if (fbdo.streamTimeout()) {
    Serial.println("Stream timeout, resuming...");
  }

  if (fbdo.streamAvailable()) {
    if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string) {
      String cmd = fbdo.to<const char*>();
      cmd.trim(); cmd.toLowerCase();
      Serial.print("Command: "); Serial.println(cmd);

      if      (cmd == "forward") Forward();
      else if (cmd == "back")    Back();
      else if (cmd == "left")    Left();
      else if (cmd == "right")   Right();
      else if (cmd == "stop")    Stop();
      else if (cmd == "center")  Stop();  // or Brake();
      else if (cmd.startsWith("speed:")) {
        int v = cmd.substring(6).toInt();
        if (v < 0) v = 0; if (v > 255) v = 255;
        CAR_SPEED = (uint8_t)v;
        Serial.printf("Speed set to %d\n", CAR_SPEED);
      } else {
        Serial.println("Unknown command");
      }
    } else {
      Serial.print("Non-string data at "); Serial.print(fbdo.dataPath());
      Serial.print(" type: "); Serial.println(fbdo.dataType());
    }
  }

  delay(5);
}
