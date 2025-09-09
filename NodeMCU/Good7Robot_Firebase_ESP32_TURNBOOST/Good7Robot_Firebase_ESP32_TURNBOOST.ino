// Good7Robot_Firebase_ESP32_FIXED.ino
// ESP32 + L298N + Firebase RTDB (Mobizt v4.4.17).
// Fixes: proper printf quoting, balanced braces, 'center' -> Forward().
//
// Libraries:
//   - Boards Manager: "ESP32 by Espressif Systems"
//   - Library Manager: "Firebase Arduino Client Library for ESP8266 and ESP32" (Mobizt) v4.4.17+
//
// Commands on /Position: forward, back, left, right, stop, center, speed:<0..255>

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// ----------------- USER SETTINGS -----------------
// Fill SSID/PASSWORD; DATABASE_URL already set to your project (regional host).
// Option A: put your RTDB Legacy Token in DATABASE_SECRET (simplest).
// Option B: leave DATABASE_SECRET empty to use API_KEY with anonymous sign-in.
static const char* WIFI_SSID       = "phoque6's Galaxy S10+";
static const char* WIFI_PASSWORD   = "rtui3148";
static const char* DATABASE_URL    = "https://nodemcu-6a1b8-wheelchair-default-rtdb.asia-southeast1.firebasedatabase.app";
static const char* DATABASE_SECRET = "fCcgpXcBfCDhSk8DAZDjj7iHngymPZ5OBXJTkD0h"; // <— put legacy token here OR leave empty to use API key
static const char* API_KEY         = "AIzaSyCbSsu4aLsGJp_WLbVRkNdtEPJNxaJkWIY"; // from your Firebase web config
// -------------------------------------------------

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String path = "/Position";

// L298N wiring (your pinout)
#define ENA_PIN 25
#define IN1     27
#define IN2     26
#define IN3     12   // strap pin; OK as OUTPUT after boot
#define IN4     14
#define ENB_PIN 13

// LEDC (PWM)
#define ENA_CH 0
#define ENB_CH 1
#define PWM_FREQ 2000  // CHANGED from 5000 to 2000 for stronger low-speed torque
#define PWM_RES  8  // 0..255

// === TURN BOOST SETTINGS (added) ===
#define TURN_SPEED      220    // higher than CAR_SPEED for stronger turns
#define SURGE_MS        120    // ms of full power at start of a turn
#define SURGE_MAX       255    // duty during surge
#define ARC_INNER_SCALE 0.55   // for arc turns (if you switch to them)

uint8_t CAR_SPEED = 150;

static inline void setSpeed(uint8_t a, uint8_t b) {
  ledcWrite(ENA_CH, a);
  ledcWrite(ENB_CH, b);
}

// Brief full-power kick to overcome static friction
static inline void surge_then(uint8_t a_after, uint8_t b_after) {
  ledcWrite(ENA_CH, SURGE_MAX);
  ledcWrite(ENB_CH, SURGE_MAX);
  delay(SURGE_MS);
  ledcWrite(ENA_CH, a_after);
  ledcWrite(ENB_CH, b_after);
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
  // TURN BOOST: pivot with surge
  surge_then(TURN_SPEED, TURN_SPEED);
  // Left motor reverse, right motor forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  Serial.println("Right (pivot)");
}


void Left() {
  // TURN BOOST: pivot with surge
  surge_then(TURN_SPEED, TURN_SPEED);
  // Left motor forward, right motor reverse
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  Serial.println("Left (pivot)");
}


void Stop() {
  setSpeed(0, 0);
  Serial.println("Stop");
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

  ledcSetup(ENA_CH, PWM_FREQ, PWM_RES);
  ledcSetup(ENB_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENA_PIN, ENA_CH);
  ledcAttachPin(ENB_PIN, ENB_CH);
  Stop();

  connectWiFi();

  // Firebase config
  config.database_url = DATABASE_URL;
  if (DATABASE_SECRET && strlen(DATABASE_SECRET) > 0) {
    config.signer.tokens.legacy_token = DATABASE_SECRET; // Legacy token auth
    Serial.println("Using RTDB Legacy Token auth");
  } else {
    config.api_key = API_KEY; // API key + anonymous sign-in
    Serial.println("Using API key + anonymous sign-in");
    if (!Firebase.signUp(&config, &auth, "", "")) {
      Serial.printf("Anonymous sign-in failed: %s\n", config.signer.signupError.message.c_str());
    }
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo, path.c_str())) {
    Serial.println(String("Stream start failed: ") + fbdo.errorReason());
  } else {
    Serial.println(String("Streaming on ") + path);
  }
}

void loop() {
  if (!Firebase.RTDB.readStream(&fbdo)) {
    Serial.println(String("Stream read failed: ") + fbdo.errorReason());
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
      else if (cmd == "center")  Forward();  // center => go straight
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
