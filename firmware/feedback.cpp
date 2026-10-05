#include "feedback.h"

void FeedbackController::begin() {
    pinMode(VIBRO_PIN, OUTPUT);
    digitalWrite(VIBRO_PIN, LOW);
}

void FeedbackController::update(bool isGoodPosture) {
    uint32_t now = millis();

    if (!isGoodPosture) {
        badPostureCounter++;
        if (badPostureCounter >= CONFIRMATION_THRESHOLD &&
            (now - lastVibroTimestamp >= VIBRO_COOLDOWN_MS) &&
            !vibrating) {
            triggerVibration();
            badPostureCounter = 0;
        }
    } else {
        badPostureCounter = 0;
    }
}

void FeedbackController::tick() {
    if (vibrating) {
        if (millis() - vibroStartTimestamp >= VIBRO_DURATION_MS) {
            stopVibration();
        }
    }
}

void FeedbackController::triggerVibration() {
    vibrating = true;
    vibroStartTimestamp = millis();
    lastVibroTimestamp = vibroStartTimestamp;
    digitalWrite(VIBRO_PIN, HIGH);
    Serial.println("ALERT: VIBRATION_ACTIVE");
}

void FeedbackController::stopVibration() {
    vibrating = false;
    digitalWrite(VIBRO_PIN, LOW);
}

bool FeedbackController::isVibrating() const {
    return vibrating;
}

uint8_t FeedbackController::getViolationCount() const {
    return badPostureCounter;
}
