#include <Arduino.h>
#include "config.h"
#include "imu.h"
#include "classifier.h"
#include "feedback.h"

IMUSensor imu;
PostureClassifier classifier;
FeedbackController feedback;

unsigned long lastInferenceTime = 0;

void setup() {
    Serial.begin(115200);

    feedback.begin();

    if (!imu.begin()) {
        Serial.println("ERR_IMU_INIT_FAILED");
        while (1) {
            delay(1000);
        }
    }

    if (!classifier.begin()) {
        Serial.println("ERR_CLASSIFIER_INIT_FAILED");
        while (1) {
            delay(1000);
        }
    }

    Serial.println("SYSTEM_READY");
}

void loop() {
    feedback.tick();

    unsigned long currentTime = millis();
    if (currentTime - lastInferenceTime >= LOOP_INTERVAL_MS) {
        lastInferenceTime = currentTime;

        AccelData accel = imu.readFiltered();
        InferenceResult result = classifier.predict(accel);

        if (result.valid) {
            feedback.update(result.isGoodPosture);

            Serial.print("X:");
            Serial.print(accel.x, 3);
            Serial.print(" Y:");
            Serial.print(accel.y, 3);
            Serial.print(" Z:");
            Serial.print(accel.z, 3);
            Serial.print(" Prob:");
            Serial.print(result.goodPostureProbability, 3);
            Serial.print(" State:");
            Serial.println(result.isGoodPosture ? "GOOD" : "BAD");
        }
    }
}