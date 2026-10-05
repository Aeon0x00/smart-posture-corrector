#pragma once
#include <Arduino.h>

constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;
constexpr uint8_t VIBRO_PIN = 4;
constexpr uint8_t MPU_ADDR = 0x68;

constexpr float MEAN_X = 0.216324f;
constexpr float MEAN_Y = 0.025815f;
constexpr float MEAN_Z = 0.363896f;
constexpr float STD_X = 0.721741f;
constexpr float STD_Y = 0.526836f;
constexpr float STD_Z = 0.428746f;

constexpr float POSTURE_PROB_THRESHOLD = 0.5f;
constexpr uint8_t CONFIRMATION_THRESHOLD = 3;
constexpr uint32_t VIBRO_DURATION_MS = 800;
constexpr uint32_t VIBRO_COOLDOWN_MS = 5000;
constexpr uint32_t LOOP_INTERVAL_MS = 500;
constexpr size_t TENSOR_ARENA_SIZE = 8192;
constexpr float EMA_ALPHA = 0.25f;
