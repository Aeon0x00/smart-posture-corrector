#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "config.h"

struct AccelReading {
    float x;
    float y;
    float z;
};

class IMUDriver {
public:
    bool begin();
    bool isConnected();
    AccelReading readAccel();

private:
    void writeReg(uint8_t reg, uint8_t data);
};
