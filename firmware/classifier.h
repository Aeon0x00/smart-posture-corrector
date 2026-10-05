#pragma once
#include <Arduino.h>
#include <Chirale_TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "config.h"
#include "imu.h"

struct InferenceResult {
    float goodPostureProbability;
    bool isGoodPosture;
    bool valid;
};

class PostureClassifier {
public:
    bool begin();
    InferenceResult predict(const AccelData& accel);

private:
    const tflite::Model* model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    tflite::AllOpsResolver resolver;
    uint8_t tensorArena[TENSOR_ARENA_SIZE];
};
