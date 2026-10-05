#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "config.h"

struct AccelData {
    float x;
    float y;
    float z;
};

class IMUSensor {
public:
    bool begin();
    AccelData readRaw();
    AccelData readFiltered();
    void calibrate(size_t samples = 64);
    bool isConnected();

private:
    float filteredX = 0.0f;
    float filteredY = 0.0f;
    float filteredZ = 0.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float offsetZ = 0.0f;
    bool initialized = false;
};
