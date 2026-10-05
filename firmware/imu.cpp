#include "imu.h"

bool IMUSensor::begin() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);
    
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    AccelData initial = readRaw();
    filteredX = initial.x;
    filteredY = initial.y;
    filteredZ = initial.z;
    initialized = true;
    return true;
}

bool IMUSensor::isConnected() {
    Wire.beginTransmission(MPU_ADDR);
    return (Wire.endTransmission() == 0);
}

AccelData IMUSensor::readRaw() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(static_cast<uint8_t>(MPU_ADDR), static_cast<size_t>(6), true);

    if (Wire.available() < 6) {
        return {filteredX, filteredY, filteredZ};
    }

    int16_t rawX = (Wire.read() << 8) | Wire.read();
    int16_t rawY = (Wire.read() << 8) | Wire.read();
    int16_t rawZ = (Wire.read() << 8) | Wire.read();

    return {
        (rawX / 16384.0f) - offsetX,
        (rawY / 16384.0f) - offsetY,
        (rawZ / 16384.0f) - offsetZ
    };
}

AccelData IMUSensor::readFiltered() {
    AccelData raw = readRaw();
    if (!initialized) {
        filteredX = raw.x;
        filteredY = raw.y;
        filteredZ = raw.z;
        initialized = true;
    } else {
        filteredX = EMA_ALPHA * raw.x + (1.0f - EMA_ALPHA) * filteredX;
        filteredY = EMA_ALPHA * raw.y + (1.0f - EMA_ALPHA) * filteredY;
        filteredZ = EMA_ALPHA * raw.z + (1.0f - EMA_ALPHA) * filteredZ;
    }
    return {filteredX, filteredY, filteredZ};
}

void IMUSensor::calibrate(size_t samples) {
    if (samples == 0) return;
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;

    for (size_t i = 0; i < samples; ++i) {
        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x3B);
        Wire.endTransmission(false);
        Wire.requestFrom(static_cast<uint8_t>(MPU_ADDR), static_cast<size_t>(6), true);

        if (Wire.available() >= 6) {
            int16_t rx = (Wire.read() << 8) | Wire.read();
            int16_t ry = (Wire.read() << 8) | Wire.read();
            int16_t rz = (Wire.read() << 8) | Wire.read();
            sumX += rx / 16384.0f;
            sumY += ry / 16384.0f;
            sumZ += rz / 16384.0f;
        }
        delay(10);
    }

    offsetX = (sumX / samples) - MEAN_X;
    offsetY = (sumY / samples) - MEAN_Y;
    offsetZ = (sumZ / samples) - MEAN_Z;
}
