#include <WiFi.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Hotspot credentials
const char* ssid = "SafeRide-Device";
const char* password = "123456789";

// Server and LED settings
WiFiServer server(3333);
const int LED_PIN = 2;

Preferences preferences;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Load previous config if needed
  loadStoredConfig();

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
    client.println("Connected with Device");

    while (client.connected()) {
      if (client.available()) {
        String req = client.readStringUntil('\n');
        Serial.print("Received From App: ");
        Serial.println(req);

        // Blink LED
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);

        // Handle JSON configuration
        if (req.startsWith("{")) {
          StaticJsonDocument<512> doc;
          DeserializationError error = deserializeJson(doc, req);

          if (!error) {
            const char* wifiSSID = doc["ssid"];
            const char* wifiPassword = doc["password"];
            const char* fullName = doc["fullName"];
            const char* contact1Name = doc["contact1Name"];
            const char* contact1Number = doc["contact1Number"];
            const char* contact2Name = doc["contact2Name"];
            const char* contact2Number = doc["contact2Number"];
            const char* contact3Name = doc["contact3Name"];
            const char* contact3Number = doc["contact3Number"];

            // Save to flash memory
            saveConfiguration(
              wifiSSID,
              wifiPassword,
              fullName,
              contact1Name,
              contact1Number,
              contact2Name,
              contact2Number,
              contact3Name,
              contact3Number
            );

            // Send confirmation to Flutter app
            client.println("done");
          } else {
            client.println("json_parse_error");
            Serial.println("Failed to parse JSON");
          }
        } else {
          String res = "From ESP32: " + req;
          client.println(res);
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}

// 🔧 Save Configuration to Flash
void saveConfiguration(
  const char* ssid,
  const char* password,
  const char* fullName,
  const char* contact1Name,
  const char* contact1Number,
  const char* contact2Name,
  const char* contact2Number,
  const char* contact3Name,
  const char* contact3Number
) {
  preferences.begin("device_config", false);

  if (ssid != nullptr)
    preferences.putString("wifi_ssid", ssid);

  if (password != nullptr)
    preferences.putString("wifi_pass", password);

  if (fullName != nullptr)
    preferences.putString("full_name", fullName);

  if (contact1Name != nullptr)
    preferences.putString("contact1_name", contact1Name);

  if (contact1Number != nullptr)
    preferences.putString("contact1_number", contact1Number);

  if (contact2Name != nullptr)
    preferences.putString("contact2_name", contact2Name);

  if (contact2Number != nullptr)
    preferences.putString("contact2_number", contact2Number);

  if (contact3Name != nullptr)
    preferences.putString("contact3_name", contact3Name);

  if (contact3Number != nullptr)
    preferences.putString("contact3_number", contact3Number);

  preferences.end();

  Serial.println("✅ Configuration saved to flash memory");
}

void loadStoredConfig() {
  preferences.begin("device_config", true);

  String wifiSSID = preferences.getString("wifi_ssid", "Not Set");
  String wifiPass = preferences.getString("wifi_pass", "Not Set");
  String fullName = preferences.getString("full_name", "Not Set");

  String contact1Name = preferences.getString("contact1_name", "Not Set");
  String contact1Number = preferences.getString("contact1_number", "Not Set");

  String contact2Name = preferences.getString("contact2_name", "Not Set");
  String contact2Number = preferences.getString("contact2_number", "Not Set");

  String contact3Name = preferences.getString("contact3_name", "Not Set");
  String contact3Number = preferences.getString("contact3_number", "Not Set");

  Serial.println("💾 Stored Configuration:");
  Serial.printf("SSID: %s\n", wifiSSID.c_str());
  Serial.printf("Password: %s\n", wifiPass.c_str());
  Serial.printf("Full Name: %s\n", fullName.c_str());
  Serial.println("Emergency Contacts:");
  Serial.printf("1. %s - %s\n", contact1Name.c_str(), contact1Number.c_str());
  Serial.printf("2. %s - %s\n", contact2Name.c_str(), contact2Number.c_str());
  Serial.printf("3. %s - %s\n", contact3Name.c_str(), contact3Number.c_str());

  preferences.end();
}