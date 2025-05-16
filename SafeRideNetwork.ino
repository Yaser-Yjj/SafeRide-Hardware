#include <WiFi.h>

// Hotspot credentials
const char* ssid = "SafeRide-Device";
const char* password = "123456789";

// Server and LED settings
WiFiServer server(3333); 
const int LED_PIN = 2; // Onboard LED is usually GPIO 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Set up Wi-Fi hotspot
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Hotspot IP: ");
  Serial.println(IP);

  // Start TCP server
  server.begin();
  Serial.println("TCP server started");
}

void loop() {
  WiFiClient client = server.available(); 

  if (client) {
    Serial.println("New client connected");

    while (client.connected()) {
      if (client.available()) {
        String req = client.readStringUntil('\n');
        Serial.print("Received From App: ");
        Serial.println(req);

        // Blink LED
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);

        String res = "From ESP32 : " + req;
        client.println(res);
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}