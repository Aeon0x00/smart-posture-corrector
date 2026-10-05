#include "indicator.h"

void Indicator::begin() {
    pixel.begin();
    pixel.clear();
    pixel.show();

    pinMode(VIBRO_PIN, OUTPUT);
    digitalWrite(VIBRO_PIN, LOW);
}

void Indicator::setColor(uint8_t r, uint8_t g, uint8_t b) {
    pixel.setPixelColor(0, pixel.Color(r, g, b));
    pixel.show();
}

void Indicator::clear() {
    pixel.clear();
    pixel.show();
}

void Indicator::blink(uint8_t r, uint8_t g, uint8_t b, uint16_t delayMs, uint8_t count) {
    for (uint8_t i = 0; i < count; ++i) {
        setColor(r, g, b);
        delay(delayMs);
        clear();
        delay(delayMs);
    }
}

void Indicator::setVibration(bool active) {
    digitalWrite(VIBRO_PIN, active ? HIGH : LOW);
}

void Indicator::vibrate(uint16_t durationMs) {
    setVibration(true);
    delay(durationMs);
    setVibration(false);
}
