#include "classifier.h"
#include "posture_model.h"

bool PostureClassifier::begin() {
    model = tflite::GetModel(posture_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        return false;
    }

    static tflite::MicroInterpreter staticInterpreter(
        model, resolver, tensorArena, sizeof(tensorArena), nullptr);
    interpreter = &staticInterpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        return false;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
    return (input != nullptr && output != nullptr);
}

InferenceResult PostureClassifier::predict(const AccelData& accel) {
    if (!interpreter || !input || !output) {
        return {0.0f, false, false};
    }

    input->data.f[0] = (accel.x - MEAN_X) / STD_X;
    input->data.f[1] = (accel.y - MEAN_Y) / STD_Y;
    input->data.f[2] = (accel.z - MEAN_Z) / STD_Z;

    if (interpreter->Invoke() != kTfLiteOk) {
        return {0.0f, false, false};
    }

    float prob = output->data.f[0];
    bool isGood = (prob >= POSTURE_PROB_THRESHOLD);
    return {prob, isGood, true};
}
