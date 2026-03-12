#include "HX711.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define DT 4
#define SCK 5
#define GREEN_LED 18
#define YELLOW_LED 19
#define RED_LED 21

HX711 scale;

const char* ssid = "Galaxy S24FE 5G";
const char* password = "Sourish2005";
const char* serverName = "http://10.185.61.186:8000/update"; 

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  scale.begin(DT, SCK);
  // TIP: If 1kg shows as 0, try changing -7050.0 to -450.0 to test sensitivity
  scale.set_scale(-7050.0); 
  scale.tare(); 

  WiFi.begin(ssid, password);
  Serial.println("\nSystem Ready.");
}

void loop() {
  // 1. Read more samples (15) for a much smoother, stable weight reading
  float weight = scale.get_units(15); 
  if (weight < 0.1) weight = 0; // Filter out noise near zero

  String status;

  // 2. STABLE LED LOGIC
  // We use wider ranges so the Yellow "Partial" state is more prominent
  if (weight < 0.5) { 
    status = "EMPTY";
    digitalWrite(GREEN_LED, HIGH); digitalWrite(YELLOW_LED, LOW); digitalWrite(RED_LED, LOW);
  } 
  else if (weight >= 0.5 && weight < 10.0) { 
    // This covers everything from a light book to a heavy backpack
    status = "PARTIALLY";
    digitalWrite(GREEN_LED, LOW); digitalWrite(YELLOW_LED, HIGH); digitalWrite(RED_LED, LOW);
  } 
  else { 
    // Anything over 10kg is considered a person/full load
    status = "FULL";
    digitalWrite(GREEN_LED, LOW); digitalWrite(YELLOW_LED, LOW); digitalWrite(RED_LED, HIGH);
  }

  // 3. LOGGING
  Serial.printf("Weight: %.2f kg | Status: %s", weight, status.c_str());

  // 4. NON-BLOCKING NETWORK UPDATE
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(1000); // 1 second max wait

    String json = "{\"seat_id\":101,\"status\":\"" + status + "\",\"weight\":" + String(weight, 2) + "}";
    int httpResponseCode = http.POST(json);

    if (httpResponseCode > 0) {
      Serial.printf(" | Server: %d\n", httpResponseCode);
    } else {
      Serial.printf(" | Offline Mode (Error: %d)\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println(" | WiFi Disconnected");
  }

  // Slight delay to allow the LEDs to be "seen" for a duration
  delay(800); 
}
