#include "imu.h"

bool IMUDriver::begin() {
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);
    delay(100);

    writeReg(MPU6050_PWR_MGMT_1, 0x00);
    delay(100);
    writeReg(MPU6050_ACCEL_CONFIG, 0x00);
    delay(50);

    return isConnected();
}

bool IMUDriver::isConnected() {
    Wire.beginTransmission(MPU6050_ADDR);
    return (Wire.endTransmission() == 0);
}

void IMUDriver::writeReg(uint8_t reg, uint8_t data) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

AccelReading IMUDriver::readAccel() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(static_cast<uint8_t>(MPU6050_ADDR), static_cast<size_t>(6), true);

    if (Wire.available() < 6) {
        return {0.0f, 0.0f, 0.0f};
    }

    int16_t ax = (Wire.read() << 8) | Wire.read();
    int16_t ay = (Wire.read() << 8) | Wire.read();
    int16_t az = (Wire.read() << 8) | Wire.read();

    return {
        ax / 16384.0f,
        ay / 16384.0f,
        az / 16384.0f
    };
}
