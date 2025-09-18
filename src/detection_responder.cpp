#include "detection_responder.h"
#include "model_settings.h"
#include "pico/stdlib.h"

// LED pin for visual feedback
#define LED_PIN PICO_DEFAULT_LED_PIN

// Initialize LED pin once
static bool is_initialized = false;

// Map confidence score to meaningful values
float ConvertToPercentage(int8_t score) {
  // This depends on your model's quantization parameters
  // For int8 quantized models, typically -128 to 127 maps to 0-1
  // Here we assume a scale of 1/256 and zero point of 0
  return (score + 128) / 255.0f * 100.0f;
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

  gpio_put(LED_PIN, 1);
  sleep_ms(10);
  gpio_put(LED_PIN, 0);
  

  // Log the detection
    // MicroPrintf("%.1f%% confidence\n", confidence);
  
//   // Visual feedback based on confidence
//   if (confidence > 70.0f) {
//     // Highly confident detection - solid LED
//     gpio_put(LED_PIN, 1);
//   } else if (confidence > 50.0f) {
//     // Medium confidence - blink LED
//     gpio_put(LED_PIN, 1);
//     sleep_ms(100);
//     gpio_put(LED_PIN, 0);
//   } else {
//     // Low confidence - LED off
//     gpio_put(LED_PIN, 0);
//   }
  
  // Add a short delay
//   sleep_ms(10);
}