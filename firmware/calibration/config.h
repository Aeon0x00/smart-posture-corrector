#pragma once
#include <Arduino.h>

constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;
constexpr uint8_t VIBRO_PIN = 4;
constexpr uint8_t BOOT_BUTTON = 0;
constexpr uint8_t LED_PIN = 21;

constexpr uint8_t MPU6050_ADDR = 0x68;
constexpr uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
constexpr uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t MPU6050_ACCEL_CONFIG = 0x1C;

constexpr uint32_t FREQUENCY = 100;
constexpr uint32_t INTERVAL_MS = 1000 / FREQUENCY;
constexpr uint16_t CALIBRATION_SAMPLES = 300;

constexpr uint32_t VIBRATION_DURATION = 800;
constexpr uint32_t VIBRATION_COOLDOWN = 5000;
constexpr uint8_t CONFIRMATION_COUNT = 3;
constexpr uint32_t LOOP_INTERVAL_MS = 500;
