#include "main_functions.h"

#include "detection_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "traffic_sign_model_data.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_log.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_interpreter.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "pico-tflmicro/src/tensorflow/lite/schema/schema_generated.h"
#include "pico/stdlib.h"
#include "pico/time.h"

// Store the last used class ID (not used in camera mode)
static uint8_t last_class_id = 255;

// Globals used for TensorFlow Lite Micro
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// Memory for model input/output tensors
constexpr int kTensorArenaSize = 136 * 1024;  // Adjust based on your model's needs
alignas(16) static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  // Map the model into a usable data structure
  model = tflite::GetModel(g_traffic_sign_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model version %d not equal to supported version %d.",
                model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Register the operations we need for AykoNet
  static tflite::MicroMutableOpResolver<10> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();
  micro_op_resolver.AddRelu();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddPad(); // For ZeroPadding2D
  micro_op_resolver.AddMean(); // For GlobalAveragePooling2D
  micro_op_resolver.AddMul();
  micro_op_resolver.AddAdd();

  // Build an interpreter to run the model
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Get information about the model's input
  input = interpreter->input(0);
  
  MicroPrintf("Model loaded successfully!");
  MicroPrintf("Input tensor shape: %d x %d x %d", 
              input->dims->data[1], 
              input->dims->data[2], 
              input->dims->data[3]);
  MicroPrintf("Waiting for camera input...\n");
}

// The name of this function is important for Arduino compatibility.
void loop() {
  // Capture image from camera
  if (kTfLiteOk != GetImage(kNumCols, kNumRows, kNumChannels, input->data.int8)) {
    MicroPrintf("Image capture failed.");
    return;
  }

  // Run the model on this input
  if (kTfLiteOk != interpreter->Invoke()) {
    MicroPrintf("Invoke failed.");
    return;
  }

  // Get the output tensor
  TfLiteTensor* output = interpreter->output(0);

  // Find the class with highest score
  int8_t max_score = -128;
  int max_index = -1;
  
  // Also find second highest for confidence assessment
  int8_t second_max_score = -128;
  
  for (int i = 0; i < kCategoryCount; i++) {
    if (output->data.int8[i] > max_score) {
      second_max_score = max_score;
      max_score = output->data.int8[i];
      max_index = i;
    } else if (output->data.int8[i] > second_max_score) {
      second_max_score = output->data.int8[i];
    }
  }
  
  // Calculate confidence gap (how much better is top prediction)
  int confidence_gap = max_score - second_max_score;
  
  // Only report if we have reasonable confidence
  // Adjust threshold based on your needs
  if (max_score > -100 && confidence_gap > 10) {
    RespondToDetection(max_index, max_score);
  } else {
    // Low confidence - unclear detection
    RespondToDetection(-1, max_score);
  }
}

// These functions are not used in camera mode but kept for compatibility
void GetPerformanceMetrics(int64_t* avg_time_us, float* accuracy) {
  *avg_time_us = 0;
  *accuracy = 0.0f;
}

void SetLastClassId(uint8_t class_id) {
  last_class_id = class_id;
}