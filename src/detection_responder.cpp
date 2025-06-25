#include "detection_responder.h"
#include "model_settings.h"
#include "pico/stdlib.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_log.h"

// LED pin for visual feedback
#define LED_PIN PICO_DEFAULT_LED_PIN

// Initialize LED pin once
static bool is_initialized = false;

// Map confidence score to meaningful values
float ConvertToPercentage(int8_t score) {
  // For int8 quantized models, convert from [-128, 127] to [0, 100]%
  // Assuming softmax output where higher values indicate higher confidence
  return ((float)(score + 128) / 255.0f) * 100.0f;
}

void RespondToDetection(int detected_class, int8_t score) {
  // Initialize LED pin first time
  if (!is_initialized) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    is_initialized = true;
  }

  // Get the class label
  const char* sign_type = 
    (detected_class >= 0 && detected_class < kCategoryCount) ? 
    kCategoryLabels[detected_class] : "Unknown";
  
  // Convert score to percentage
  float confidence = ConvertToPercentage(score);

  // Only report detections above a certain confidence threshold
  if (confidence >99.0f) {
    MicroPrintf("DETECTED: %s (%.1f%% confidence)", sign_type, confidence);
    
    // Visual feedback - blink LED for high-confidence detections
    gpio_put(LED_PIN, 1);
    sleep_ms(50);
    gpio_put(LED_PIN, 0);
  }
  // } else if (confidence > 80.0f) {
  //   // Medium confidence - just log without LED
  //   MicroPrintf("Possible: %s (%.1f%%)", sign_type, confidence);
  // }
  // Low confidence detections are ignored to reduce noise
}