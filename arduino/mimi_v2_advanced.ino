#include "HX711.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h> // Include WiFiManager Library

// --- CONFIGURATION ---
// No hardcoded WiFi credentials needed!
const char* defaultServerName = "http://your-app-name.render.com/update"; 

float calibration_factor = -450.0; 

// Pins - Weight System
#define LOADCELL_DOUT_PIN 4
#define LOADCELL_SCK_PIN 5
#define W_LED_G 18
#define W_LED_Y 19
#define W_LED_R 21

// Pins - Distance System
#define TRIG_PIN 12
#define ECHO_PIN 14
#define D_LED_G 25
#define D_LED_Y 26
#define D_LED_R 27

HX711 scale;
HTTPClient http;

unsigned long lastUpdateTime = 0;
const long updateInterval = 500;  

// Moving average buffer
float weightBuffer[5];
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);

  pinMode(W_LED_G, OUTPUT); pinMode(W_LED_Y, OUTPUT); pinMode(W_LED_R, OUTPUT);
  pinMode(D_LED_G, OUTPUT); pinMode(D_LED_Y, OUTPUT); pinMode(D_LED_R, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);

  Serial.println("Stabilizing Loadcell...");
  delay(1000);   
  scale.tare();
  Serial.println("Tare complete.");

  // --- WiFiManager Setup ---
  WiFiManager wm;
  
  // Uncomment to reset settings for testing
  // wm.resetSettings();

  Serial.println("Connecting to WiFi...");
  // This will create a hotspot named "OccuTrack_Setup" if it fails to connect
  bool res = wm.autoConnect("OccuTrack_Setup"); 

  if (!res) {
    Serial.println("Failed to connect. Restarting...");
    ESP.restart();
  } else {
    Serial.println("Connected to WiFi! Ready.");
  }
}

float getSmoothedWeight() {
  float raw = scale.get_units(5);
  if (raw < 0) raw = 0;
  weightBuffer[bufferIndex] = raw;
  bufferIndex = (bufferIndex + 1) % 5;
  float sum = 0;
  for (int i = 0; i < 5; i++) sum += weightBuffer[i];
  return sum / 5.0;
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duration == 0) return 400.0;
  return (duration * 0.034) / 2;
}

void loop() {
  float weight = getSmoothedWeight();
  float distance = getDistance();

  if (weight < 0.15) weight = 0.0;  // ignore tiny noise
  if (weight > 20.0) weight = 20.0;

  String w_status;

  if (weight < 0.50) {
    w_status = "EMPTY";
    digitalWrite(W_LED_G, HIGH); digitalWrite(W_LED_Y, LOW); digitalWrite(W_LED_R, LOW);
  } else if (weight < 5.0) {
    w_status = "PARTIALLY";
    digitalWrite(W_LED_G, LOW); digitalWrite(W_LED_Y, HIGH); digitalWrite(W_LED_R, LOW);
  } else {
    w_status = "FULL";
    digitalWrite(W_LED_G, LOW); digitalWrite(W_LED_Y, LOW); digitalWrite(W_LED_R, HIGH);
  }

  // Distance Visuals
  if (distance > 100) {
    digitalWrite(D_LED_G, HIGH); digitalWrite(D_LED_Y, LOW); digitalWrite(D_LED_R, LOW);
  } else if (distance > 40) {
    digitalWrite(D_LED_G, LOW); digitalWrite(D_LED_Y, HIGH); digitalWrite(D_LED_R, LOW);
  } else {
    digitalWrite(D_LED_G, LOW); digitalWrite(D_LED_Y, LOW); digitalWrite(D_LED_R, HIGH);
  }

  // ---- SERVER UPDATE ----
  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();

    if (WiFi.status() == WL_CONNECTED) {
      http.begin(defaultServerName);
      http.addHeader("Content-Type", "application/json");

      String json = "{\"seat_id\":101,\"status\":\"" + w_status +
                    "\",\"weight\":" + String(weight, 2) +
                    ",\"dist\":" + String(distance, 1) + "}";

      int httpResponseCode = http.POST(json);
      if (httpResponseCode > 0) {
        Serial.printf("Data Sent! Status: %d\n", httpResponseCode);
      } else {
        Serial.printf("Error sending data: %s\n", http.errorToString(httpResponseCode).c_str());
      }
      http.end();
    }
    Serial.printf("W: %.2f kg | D: %.1f cm | %s\n", weight, distance, w_status.c_str());
  }
}
