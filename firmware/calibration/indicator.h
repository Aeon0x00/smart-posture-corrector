#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

class Indicator {
public:
    void begin();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void clear();
    void blink(uint8_t r, uint8_t g, uint8_t b, uint16_t delayMs, uint8_t count);
    void setVibration(bool active);
    void vibrate(uint16_t durationMs);

private:
    Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
};
