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
#include "pico/time.h" // For absolute_time functions

// Global variables for tracking performance metrics
static int64_t total_inference_time = 0;
static int correct_predictions = 0;
static int total_predictions = 0;

// Expose image_index from image_provider.cpp
extern uint32_t image_index;

// Store the last used class ID for comparison with prediction
static uint8_t last_class_id = 0;

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
    MicroPrintf("Model version %d not equal to supported version %d.",model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Register the operations we need for AykocNet
  static tflite::MicroMutableOpResolver<10> micro_op_resolver; // Adjusted number of ops
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
  
  // Log the input tensor shape
  // MicroPrintf("Input tensor dimensions: %d x %d x %d", input->dims->data[1], input->dims->data[2], input->dims->data[3]);
  
  // Initialize performance metrics
  total_inference_time = 0;
  correct_predictions = 0;
  total_predictions = 0;
}

// The name of this function is important for Arduino compatibility.
void loop() {
  // Capture or load an image - this will update the image_index and store the class internally
  if (kTfLiteOk != GetImage(kNumCols, kNumRows, kNumChannels, input->data.int8)) {
    MicroPrintf("Image capture failed.");
    return;
  }
  
  // Get the true class ID from the image provider
  // This works because when GetImage is called, it internally stores the class ID we need
  uint8_t true_class = last_class_id;

  // Start timing
  absolute_time_t start_time = get_absolute_time();
  
  // Run the model on this input
  if (kTfLiteOk != interpreter->Invoke()) {
    MicroPrintf("Invoke failed.");
    return;
  }
  
  // End timing
  absolute_time_t end_time = get_absolute_time();
  int64_t inference_time_us = absolute_time_diff_us(start_time, end_time);
  
  // Accumulate inference time
  total_inference_time += inference_time_us;
  total_predictions++;

  // Get the output tensor
  TfLiteTensor* output = interpreter->output(0);

  // Find the class with highest score
  int8_t max_score = -128;
  int max_index = -1;
  for (int i = 0; i < kCategoryCount; i++) {
    if (output->data.int8[i] > max_score) {
      max_score = output->data.int8[i];
      max_index = i;
    }
  }
  
  // Check if the prediction is correct
  if (max_index == true_class) {
    correct_predictions++;
    // MicroPrintf("CORRECT\n");
  } else {
    MicroPrintf("WRONG: Expected class %d but got %d.\n", 
                true_class, max_index);
  }

  // Process the results
  RespondToDetection(max_index, max_score);
}

// Implementation of performance metrics function
void GetPerformanceMetrics(int64_t* avg_time_us, float* accuracy) {
  if (total_predictions > 0) {
    *avg_time_us = total_inference_time / total_predictions;
    *accuracy = (float)correct_predictions / total_predictions * 100.0f;
  } else {
    *avg_time_us = 0;
    *accuracy = 0.0f;
  }
}

// Add this function to get class ID for accuracy calculation
void SetLastClassId(uint8_t class_id) {
  last_class_id = class_id;
}