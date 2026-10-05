#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct CalibrationData {
    float baselineX = 0.0f;
    float baselineY = 0.0f;
    float baselineZ = 0.0f;
    float threshold = 0.3f;
    float sensitivity = 1.0f;
    bool calibrated = false;
};

class CalibrationStorage {
public:
    void load(CalibrationData& data);
    void save(const CalibrationData& data);

private:
    Preferences preferences;
};
