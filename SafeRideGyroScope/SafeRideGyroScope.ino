#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Buzzer pin
const int BUZZER_PIN = 15;
const int LED_PIN = 2;

float yawDeg = 0;
float gyroZOffset = 0;
unsigned long lastIMU = 0;
const float DEG2RAD = M_PI / 180.0f;

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize I2C
  Wire.begin();

  // Initialize MPU6050
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

  // Setup buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // 🔊 Buzzer OFF by default

  digitalWrite(LED_PIN, LOW);

  Serial.println("MPU6050 Calibration Done ✔");

}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;

  // Read raw values
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Compute delta time
  unsigned long now = millis();
  float dt = (now - lastIMU) / 1000.0f;
  lastIMU = now;

  // Update yaw (rotation around Z-axis)
  yawDeg += (gz - gyroZOffset) / 131.0f * dt;

  // Print yaw to serial monitor
  Serial.print("Yaw: ");
  Serial.print(yawDeg);
  Serial.println("°");

  // Optional: Turn buzzer ON if yaw > threshold
  if (yawDeg > 65 || yawDeg < -65) {
    digitalWrite(BUZZER_PIN, 0);  // 🔔 Turn on buzzer
    digitalWrite(LED_PIN, 1);
  } else {
    digitalWrite(BUZZER_PIN, 0);  // 🔇 Turn off buzzer
    digitalWrite(LED_PIN, LOW);
  }
}
