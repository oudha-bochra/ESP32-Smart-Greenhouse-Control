#define BLYNK_TEMPLATE_ID "TMPL2iFjO9qBN"
#define BLYNK_TEMPLATE_NAME "Smart system"
#define BLYNK_AUTH_TOKEN "PZ6iAlIssj3cbD9FTWKwW5uX9mz8F1II"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT22
#define TRIG_PIN 5     // Ultrasonic Trig Pin for Water Tank
#define ECHO_PIN 18    // Ultrasonic Echo Pin for Water Tank
#define LDR_PIN 34     // LDR connected to GPIO 34
#define LED_GREEN 2
#define LED_RED 4
#define SPRAYER_PIN 19 // Sprayer Pump Relay
#define FAN_PIN 23     // Exhaust Fan Relay
#define BUZZER_PIN 13

const float GAMMA = 0.7;
const float RL10 = 33.0;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

// Function to read LDR Sensor precisely (GPIO 34)
float readLux() {
  int analogValue = analogRead(LDR_PIN);
  float voltage = analogValue / 4096.0 * 3.3;
  if (voltage >= 3.29) voltage = 3.29;

  float resistance = 2000.0 * voltage / (1.0 - voltage / 3.3);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1.0 / GAMMA));
  return lux;
}

// Function to read Water Tank Level (0-100%)
int readWaterTank() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 100;

  int distance = duration * 0.034 / 2; // Distance in cm
  int tankLevel = map(distance, 25, 5, 0, 100);
  tankLevel = constrain(tankLevel, 0, 100);
  return tankLevel;
}

void monitorGreenhouseV3() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = readLux();
  int waterLevel = readWaterTank();

  if (isnan(h) || isnan(t)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!! ");
    return;
  }

  String statusStr = "";
  bool isWarning = false;

  // Debug Serial
  Serial.print("Lux: "); Serial.print(lux);
  Serial.print(" | Hum: "); Serial.print(h);
  Serial.print(" | Tank: "); Serial.print(waterLevel); Serial.println("%");

  // SYSTEM LOGIC
  // 1. Empty Water Tank Protection
  if (waterLevel <= 15) {
    digitalWrite(SPRAYER_PIN, LOW); // Force pump OFF
    statusStr = "TANK EMPTY!";
    isWarning = true;
    tone(BUZZER_PIN, 1500, 200);
    Blynk.logEvent("tank_kosong", "WARNING! Water Tank Empty [" + String(waterLevel) + "%], Pump Turned OFF!");
  } else {
    // 2. Humidity Sprayer Control
    if (h < 70.0) {
      digitalWrite(SPRAYER_PIN, HIGH);
      statusStr = "SPRAYING...";
      isWarning = true;
    } else if (h >= 80.0) {
      digitalWrite(SPRAYER_PIN, LOW);
    }
  }

  // 3. Fan Control
  if (t > 28.0) {
    digitalWrite(FAN_PIN, HIGH);
    if (statusStr == "") statusStr = "HOT! FAN ON";
    isWarning = true;
  } else {
    digitalWrite(FAN_PIN, LOW);
  }

  // 4. Light Intensity Protection (> 50 Lux)
  if (lux > 50.0) {
    statusStr = "TOO BRIGHT!";
    isWarning = true;
    tone(BUZZER_PIN, 2000, 100);
  }

  if (!isWarning && waterLevel > 15) {
    statusStr = "ALL SYSTEM SAFE";
    noTone(BUZZER_PIN);
  }

  // LED Indicators
  if (isWarning) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  }

  // Blynk Update
  Blynk.virtualWrite(V0, h);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, statusStr);
  Blynk.virtualWrite(V3, digitalRead(SPRAYER_PIN) ? 1 : 0);
  Blynk.virtualWrite(V4, digitalRead(FAN_PIN) ? 1 : 0);
  Blynk.virtualWrite(V5, lux);
  Blynk.virtualWrite(V6, waterLevel);

  // LCD Update
  lcd.setCursor(0, 0);
  lcd.print("H:"); lcd.print(h, 0); lcd.print("% L:"); lcd.print(lux, 0); lcd.print("Lx  ");

  lcd.setCursor(0, 1);
  lcd.print("ST:");
  lcd.print(statusStr);
  lcd.print("        ");
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(SPRAYER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.print("Bochra System");
  lcd.setCursor(0, 1);
  lcd.print("WS Tank & Lux");
  delay(1500);

  Blynk.begin(auth, ssid, pass);
  lcd.clear();

  timer.setInterval(500L, monitorGreenhouseV3);
}

void loop() {
  Blynk.run();
  timer.run();
}