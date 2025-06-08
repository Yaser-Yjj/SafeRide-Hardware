#include <WiFi.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Wire.h>
#include <MPU6050.h>

// MPU6050 + Gyro Setup
MPU6050 mpu;
float yawDeg = 0;
float gyroZOffset = 0;
unsigned long lastIMU = 0;
const float DEG2RAD = M_PI / 180.0f;

// Buzzer and LED pins
const int BUZZER_PIN = 15;
const int LED_PIN = 2;

// Wi-Fi Hotspot
const char* ssid = "SafeRide-Device";
const char* password = "123456789";
WiFiServer server(3333);
Preferences preferences;

void setup() {
  Serial.begin(115200);

  setupMpu();

  loadStoredConfig();

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Hotspot IP: ");
  Serial.println(IP);

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
            const char* contact3Number = doc["contact3Name"];

            saveConfiguration(
              wifiSSID,
              wifiPassword,
              fullName,
              contact1Name,
              contact1Number,
              contact2Name,
              contact2Number,
              contact3Name,
              contact3Number);

            client.println("done");
          } else {
            client.println("json_parse_error");
            Serial.println("Failed to parse JSON");
          }
        }

        // Echo back received message
        else if (req.length() > 0) {
          String res = "ESP32 received: " + req;
          Serial.println(req);
          client.println(res);
        }
      }

      float currentYaw = DisplayDegre();

      // Print yaw to serial monitor
      Serial.print("Yaw: ");
      Serial.print(currentYaw);
      Serial.println("°");
      
      // Optional: Turn buzzer ON if yaw exceeds threshold
      if (currentYaw > 65 || currentYaw < -65) {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, HIGH);

        if (client && client.connected()) {
          client.println("accident");
        }
      } else {
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
      }

    }

    client.stop();
    Serial.println("Client disconnected");
  }
}


void setupMpu() {
  Wire.begin();
  mpu.initialize();

  // Calibrate Gyro Z-axis offset
  long sum = 0;
  for (int i = 0; i < 200; i++) {
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);
    sum += gz;
    delay(10);
  }
  gyroZOffset = sum / 200.0f;
  lastIMU = millis();

  // Setup buzzer & LED
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Default OFF
  digitalWrite(LED_PIN, LOW);

  Serial.println("MPU6050 Calibration Done ✔");
}

float DisplayDegre() {
  int16_t ax, ay, az, gx, gy, gz;

  // Read raw values
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Compute delta time
  unsigned long now = millis();
  float dt = (now - lastIMU) / 1000.0f;
  lastIMU = now;

  // Update yaw (rotation around Z-axis)
  yawDeg += (gz - gyroZOffset) / 131.0f * dt;

  return yawDeg;
}

void saveConfiguration(
  const char* wifiSSID,
  const char* wifiPassword,
  const char* fullName,
  const char* contact1Name,
  const char* contact1Number,
  const char* contact2Name,
  const char* contact2Number,
  const char* contact3Name,
  const char* contact3Number) {

  preferences.begin("device_config", false);

  if (wifiSSID != nullptr)
    preferences.putString("wifi_ssid", wifiSSID);

  if (wifiPassword != nullptr)
    preferences.putString("wifi_pass", wifiPassword);

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