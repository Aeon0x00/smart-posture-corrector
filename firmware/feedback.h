#pragma once
#include <Arduino.h>
#include "config.h"

class FeedbackController {
public:
    void begin();
    void update(bool isGoodPosture);
    void tick();
    bool isVibrating() const;
    uint8_t getViolationCount() const;

private:
    void triggerVibration();
    void stopVibration();

    uint8_t badPostureCounter = 0;
    uint32_t lastVibroTimestamp = 0;
    uint32_t vibroStartTimestamp = 0;
    bool vibrating = false;
};
