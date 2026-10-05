#include "calibration_storage.h"

void CalibrationStorage::load(CalibrationData& data) {
    preferences.begin("posture", false);
    data.baselineX = preferences.getFloat("base_x", 0.0f);
    data.baselineY = preferences.getFloat("base_y", 0.0f);
    data.baselineZ = preferences.getFloat("base_z", 0.0f);
    data.threshold = preferences.getFloat("threshold", 0.3f);
    data.sensitivity = preferences.getFloat("sensitivity", 1.0f);
    data.calibrated = preferences.getBool("calibrated", false);
    preferences.end();
}

void CalibrationStorage::save(const CalibrationData& data) {
    preferences.begin("posture", false);
    preferences.putFloat("base_x", data.baselineX);
    preferences.putFloat("base_y", data.baselineY);
    preferences.putFloat("base_z", data.baselineZ);
    preferences.putFloat("threshold", data.threshold);
    preferences.putFloat("sensitivity", data.sensitivity);
    preferences.putBool("calibrated", true);
    preferences.end();
}
