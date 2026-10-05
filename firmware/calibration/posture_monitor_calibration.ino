#include <Arduino.h>
#include "config.h"
#include "imu.h"
#include "indicator.h"
#include "calibration_storage.h"

IMUDriver imu;
Indicator indicator;
CalibrationStorage storage;
CalibrationData calib;

unsigned long lastVibrationTime = 0;
int badPostureCounter = 0;

void performCalibration() {
    indicator.blink(0, 0, 255, 250, 6);
    delay(3000);
    indicator.setColor(0, 0, 255);

    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;

    for (int i = 0; i < CALIBRATION_SAMPLES; ++i) {
        AccelReading reading = imu.readAccel();
        sumX += reading.x;
        sumY += reading.y;
        sumZ += reading.z;
        delay(10);
    }

    calib.baselineX = sumX / CALIBRATION_SAMPLES;
    calib.baselineY = sumY / CALIBRATION_SAMPLES;
    calib.baselineZ = sumZ / CALIBRATION_SAMPLES;
    calib.threshold = 0.3f;
    calib.sensitivity = 1.0f;
    calib.calibrated = true;

    storage.save(calib);

    indicator.setColor(0, 255, 0);
    delay(2000);
    indicator.clear();
    delay(1000);
}

void setup() {
    Serial.begin(115200);

    indicator.begin();

    if (!imu.begin()) {
        while (1) {
            indicator.blink(255, 0, 0, 500, 1);
        }
    }

    pinMode(BOOT_BUTTON, INPUT_PULLUP);

    indicator.vibrate(300);

    storage.load(calib);

    if (digitalRead(BOOT_BUTTON) == LOW) {
        delay(500);
        performCalibration();
    }

    if (!calib.calibrated) {
        indicator.blink(255, 255, 0, 300, 4);
    }
}

void loop() {
    if (digitalRead(BOOT_BUTTON) == LOW) {
        delay(50);
        if (digitalRead(BOOT_BUTTON) == LOW) {
            performCalibration();
        }
    }

    AccelReading acc = imu.readAccel();
    float deviation = 0.0f;

    if (calib.calibrated) {
        float diffX = fabsf(acc.x - calib.baselineX);
        float diffY = fabsf(acc.y - calib.baselineY);
        float diffZ = fabsf(acc.z - calib.baselineZ);
        deviation = (diffX * 0.3f + diffY * 0.4f + diffZ * 0.3f) * calib.sensitivity;
    } else {
        deviation = fabsf(acc.y) + fabsf(acc.z);
    }

    unsigned long currentTime = millis();
    bool cooldownActive = (currentTime - lastVibrationTime) < VIBRATION_COOLDOWN;

    if (deviation > calib.threshold) {
        badPostureCounter++;
        indicator.setColor(255, 0, 0);

        if (badPostureCounter >= CONFIRMATION_COUNT && !cooldownActive) {
            indicator.vibrate(VIBRATION_DURATION);
            lastVibrationTime = currentTime;
            badPostureCounter = 0;
        }
    } else {
        badPostureCounter = 0;
        indicator.setColor(0, 50, 0);
    }

    Serial.print("X:");
    Serial.print(acc.x, 3);
    Serial.print(" Y:");
    Serial.print(acc.y, 3);
    Serial.print(" Z:");
    Serial.print(acc.z, 3);
    Serial.print(" Dev:");
    Serial.print(deviation, 3);
    Serial.print(" Bad:");
    Serial.println(badPostureCounter);

    delay(LOOP_INTERVAL_MS);
}
