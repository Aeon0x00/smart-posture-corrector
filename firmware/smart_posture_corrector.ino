/*
 * Устройство для коррекции осанки
 * Курсовая работа
 */

#include <Wire.h>
#include <Chirale_TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "posture_model.h"

// Пины
#define SDA_PIN 8
#define SCL_PIN 9
#define VIBRO_PIN 4
#define MPU_ADDR 0x68

// Параметры нормализации (из обучения)
#define MEAN_X 0.216324f
#define MEAN_Y 0.025815f
#define MEAN_Z 0.363896f
#define STD_X 0.721741f
#define STD_Y 0.526836f
#define STD_Z 0.428746f

// Настройки
float threshold = 0.6;
int confirm_count = 3;
int vibro_time = 800;
int cooldown = 5000;

// TFLite
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
uint8_t tensor_arena[8192];

int bad_count = 0;
unsigned long last_vibro = 0;

void setup() {
  Serial.begin(115200);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  // MPU-6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  // Вибромотор
  pinMode(VIBRO_PIN, OUTPUT);

  // Загрузка модели
  model = tflite::GetModel(posture_model);
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, sizeof(tensor_arena), nullptr);
  interpreter = &static_interpreter;
  interpreter->AllocateTensors();

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("Ready");
}

void loop() {
  // Чтение акселерометра
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();

  float x = ax / 16384.0f;
  float y = ay / 16384.0f;
  float z = az / 16384.0f;

  // Нормализация
  input->data.f[0] = (x - MEAN_X) / STD_X;
  input->data.f[1] = (y - MEAN_Y) / STD_Y;
  input->data.f[2] = (z - MEAN_Z) / STD_Z;

  // Инференс
  interpreter->Invoke();

  // Модель выдаёт 1 выход: вероятность хорошей осанки (0-1)
  float good_prob = output->data.f[0];
  bool good_posture = (good_prob > 0.5);

  // Вывод
  Serial.print("X:");
  Serial.print(x);
  Serial.print(" Y:");
  Serial.print(y);
  Serial.print(" Z:");
  Serial.print(z);
  Serial.print(" Good:");
  Serial.print(good_prob);
  Serial.print(" Posture:");
  Serial.println(good_posture ? "GOOD" : "BAD");

  // Проверка осанки
  if (!good_posture) {
    bad_count++;
    if (bad_count >= confirm_count &&
        (millis() - last_vibro) > cooldown) {
      digitalWrite(VIBRO_PIN, HIGH);
      delay(vibro_time);
      digitalWrite(VIBRO_PIN, LOW);
      last_vibro = millis();
      bad_count = 0;
      Serial.println("VIBRO!");
    }
  } else {
    bad_count = 0;
  }

  delay(500);
}