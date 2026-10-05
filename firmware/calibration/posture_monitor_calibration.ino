/*
 * IoT-устройство для коррекции осанки с персонализированной калибровкой
 *
 * Компоненты:
 * - ESP32-S3 Zero (Waveshare)
 * - MPU-6050 (акселерометр + гироскоп)
 * - Вибромотор на GPIO4
 * - Кнопка BOOT (GPIO0) для калибровки
 * - Светодиод WS2812 (GPIO21) для индикации
 *
 * Режимы работы:
 * 1. Нормальный режим - мониторинг осанки с вибрацией
 * 2. Режим калибровки - настройка под конкретного пользователя
 */

#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>

// Пины
#define SDA_PIN 8
#define SCL_PIN 9
#define VIBRO_PIN 4
#define BOOT_BUTTON 0
#define LED_PIN 21

// MPU-6050
#define MPU6050_ADDR 0x68
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_CONFIG 0x1C

// Параметры
const int FREQUENCY = 100;  // 100 Гц
const int INTERVAL_MS = 1000 / FREQUENCY;
const int CALIBRATION_SAMPLES = 300;  // 3 секунды калибровки

// Светодиод
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// Хранилище настроек
Preferences preferences;

// Калибровочные данные
struct CalibrationData {
  float baseline_x;
  float baseline_y;
  float baseline_z;
  float threshold;
  float sensitivity;
  bool calibrated;
};

CalibrationData calib;

// Параметры вибрации
const int VIBRATION_DURATION = 800;
const int VIBRATION_COOLDOWN = 5000;
unsigned long lastVibrationTime = 0;
int bad_posture_counter = 0;
const int CONFIRMATION_COUNT = 3;

// ============================================
// MPU-6050 функции
// ============================================

void mpu6050_write(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void mpu6050_read_accel(int16_t *ax, int16_t *ay, int16_t *az) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);

  *ax = (Wire.read() << 8) | Wire.read();
  *ay = (Wire.read() << 8) | Wire.read();
  *az = (Wire.read() << 8) | Wire.read();
}

// ============================================
// Калибровка
// ============================================

void loadCalibration() {
  preferences.begin("posture", false);

  calib.baseline_x = preferences.getFloat("base_x", 0.0);
  calib.baseline_y = preferences.getFloat("base_y", 0.0);
  calib.baseline_z = preferences.getFloat("base_z", 0.0);
  calib.threshold = preferences.getFloat("threshold", 0.3);
  calib.sensitivity = preferences.getFloat("sensitivity", 1.0);
  calib.calibrated = preferences.getBool("calibrated", false);

  preferences.end();

  if (calib.calibrated) {
    Serial.println("✅ Loaded calibration from memory");
    Serial.printf("Baseline: X=%.3f Y=%.3f Z=%.3f\n",
                  calib.baseline_x, calib.baseline_y, calib.baseline_z);
    Serial.printf("Threshold: %.3f, Sensitivity: %.2f\n",
                  calib.threshold, calib.sensitivity);
  } else {
    Serial.println("⚠️  No calibration found - using defaults");
  }
}

void saveCalibration() {
  preferences.begin("posture", false);

  preferences.putFloat("base_x", calib.baseline_x);
  preferences.putFloat("base_y", calib.baseline_y);
  preferences.putFloat("base_z", calib.baseline_z);
  preferences.putFloat("threshold", calib.threshold);
  preferences.putFloat("sensitivity", calib.sensitivity);
  preferences.putBool("calibrated", true);

  preferences.end();

  Serial.println("✅ Calibration saved to memory");
}

void performCalibration() {
  Serial.println("\n" + String("=").repeat(50));
  Serial.println("🎯 CALIBRATION MODE");
  Serial.println(String("=").repeat(50));
  Serial.println("\n📋 Instructions:");
  Serial.println("1. Sit with GOOD POSTURE (straight back)");
  Serial.println("2. Stay still for 3 seconds");
  Serial.println("3. Device will record your baseline\n");

  // Индикация: синий мигающий
  for (int i = 0; i < 6; i++) {
    led.setPixelColor(0, led.Color(0, 0, 255));
    led.show();
    delay(250);
    led.clear();
    led.show();
    delay(250);
  }

  Serial.println("⏳ Recording baseline in 3...");
  delay(1000);
  Serial.println("⏳ 2...");
  delay(1000);
  Serial.println("⏳ 1...");
  delay(1000);
  Serial.println("📊 Recording...");

  // Светодиод: постоянный синий
  led.setPixelColor(0, led.Color(0, 0, 255));
  led.show();

  // Сбор данных
  float sum_x = 0, sum_y = 0, sum_z = 0;

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    int16_t ax, ay, az;
    mpu6050_read_accel(&ax, &ay, &az);

    sum_x += ax / 16384.0f;
    sum_y += ay / 16384.0f;
    sum_z += az / 16384.0f;

    delay(10);  // 100 Гц

    // Прогресс
    if (i % 50 == 0) {
      Serial.print(".");
    }
  }

  Serial.println();

  // Вычисление среднего
  calib.baseline_x = sum_x / CALIBRATION_SAMPLES;
  calib.baseline_y = sum_y / CALIBRATION_SAMPLES;
  calib.baseline_z = sum_z / CALIBRATION_SAMPLES;
  calib.threshold = 0.3;  // Начальный порог
  calib.sensitivity = 1.0;
  calib.calibrated = true;

  // Сохранение
  saveCalibration();

  Serial.println("\n✅ CALIBRATION COMPLETE!");
  Serial.printf("Baseline recorded: X=%.3f Y=%.3f Z=%.3f\n",
                calib.baseline_x, calib.baseline_y, calib.baseline_z);

  // Индикация: зелёный 2 секунды
  led.setPixelColor(0, led.Color(0, 255, 0));
  led.show();
  delay(2000);
  led.clear();
  led.show();

  Serial.println("\n🚀 Returning to normal mode...\n");
  delay(1000);
}

// ============================================
// Setup
// ============================================

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n" + String("=").repeat(50));
  Serial.println("🧠 POSTURE MONITOR WITH CALIBRATION");
  Serial.println(String("=").repeat(50));

  // Инициализация светодиода
  led.begin();
  led.clear();
  led.show();

  // Инициализация I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  delay(100);

  // Инициализация MPU-6050
  mpu6050_write(MPU6050_PWR_MGMT_1, 0x00);
  delay(100);
  mpu6050_write(MPU6050_ACCEL_CONFIG, 0x00);
  delay(50);

  // Проверка подключения
  Wire.beginTransmission(MPU6050_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println("✅ MPU-6050 connected");
  } else {
    Serial.println("❌ ERROR: MPU-6050 not found!");
    while (1) {
      led.setPixelColor(0, led.Color(255, 0, 0));
      led.show();
      delay(500);
      led.clear();
      led.show();
      delay(500);
    }
  }

  // Инициализация вибромотора
  pinMode(VIBRO_PIN, OUTPUT);
  digitalWrite(VIBRO_PIN, LOW);

  // Инициализация кнопки
  pinMode(BOOT_BUTTON, INPUT_PULLUP);

  // Тестовая вибрация
  Serial.println("🔊 Testing vibration motor...");
  digitalWrite(VIBRO_PIN, HIGH);
  delay(300);
  digitalWrite(VIBRO_PIN, LOW);

  // Загрузка калибровки
  loadCalibration();

  // Проверка кнопки при старте
  if (digitalRead(BOOT_BUTTON) == LOW) {
    Serial.println("\n🔧 BOOT button pressed - entering calibration mode");
    delay(500);
    performCalibration();
  }

  if (!calib.calibrated) {
    Serial.println("\n⚠️  WARNING: Device not calibrated!");
    Serial.println("💡 Hold BOOT button and reset to calibrate");
    // Индикация: жёлтый мигающий
    for (int i = 0; i < 4; i++) {
      led.setPixelColor(0, led.Color(255, 255, 0));
      led.show();
      delay(300);
      led.clear();
      led.show();
      delay(300);
    }
  }

  Serial.println("\n🚀 System ready!");
  Serial.println("Press BOOT button anytime to recalibrate\n");
  Serial.println(String("=").repeat(50));
  Serial.println();
}

// ============================================
// Loop
// ============================================

void loop() {
  // Проверка кнопки калибровки
  if (digitalRead(BOOT_BUTTON) == LOW) {
    delay(50);  // Дебаунс
    if (digitalRead(BOOT_BUTTON) == LOW) {
      performCalibration();
    }
  }

  // Чтение акселерометра
  int16_t ax, ay, az;
  mpu6050_read_accel(&ax, &ay, &az);

  float acc_x = ax / 16384.0f;
  float acc_y = ay / 16384.0f;
  float acc_z = az / 16384.0f;

  // Вычисление отклонения от baseline
  float deviation = 0;

  if (calib.calibrated) {
    float diff_x = abs(acc_x - calib.baseline_x);
    float diff_y = abs(acc_y - calib.baseline_y);
    float diff_z = abs(acc_z - calib.baseline_z);

    // Взвешенное отклонение (Y и Z более важны для осанки)
    deviation = (diff_x * 0.3 + diff_y * 0.4 + diff_z * 0.3) * calib.sensitivity;
  } else {
    // Без калибровки - используем простой порог
    deviation = abs(acc_y) + abs(acc_z);
  }

  // Вывод данных
  Serial.print("Accel: X=");
  Serial.print(acc_x, 3);
  Serial.print(" Y=");
  Serial.print(acc_y, 3);
  Serial.print(" Z=");
  Serial.print(acc_z, 3);
  Serial.print(" | Deviation: ");
  Serial.print(deviation, 3);
  Serial.print(" | ");

  // Логика определения плохой осанки
  unsigned long current_time = millis();
  bool cooldown_active = (current_time - lastVibrationTime) < VIBRATION_COOLDOWN;

  if (deviation > calib.threshold) {
    bad_posture_counter++;
    Serial.print("⚠️  BAD (");
    Serial.print(bad_posture_counter);
    Serial.print("/");
    Serial.print(CONFIRMATION_COUNT);
    Serial.println(")");

    // Индикация: красный
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();

    if (bad_posture_counter >= CONFIRMATION_COUNT && !cooldown_active) {
      Serial.println(">>> VIBRATING! <<<");
      digitalWrite(VIBRO_PIN, HIGH);
      delay(VIBRATION_DURATION);
      digitalWrite(VIBRO_PIN, LOW);
      lastVibrationTime = current_time;
      bad_posture_counter = 0;
    }
  } else {
    bad_posture_counter = 0;
    Serial.println("✓ Good");

    // Индикация: зелёный
    led.setPixelColor(0, led.Color(0, 50, 0));
    led.show();
  }

  delay(500);  // Обновление 2 раза в секунду
}
