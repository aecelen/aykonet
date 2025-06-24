#include "detection_responder.h"
#include "model_settings.h"
#include "pico/stdlib.h"

// LED pin for visual feedback
#define LED_PIN PICO_DEFAULT_LED_PIN

// Initialize LED pin once
static bool is_initialized = false;

// Track last detection for change notification
static int last_detected_class = -1;
static int same_class_count = 0;

// Map confidence score to meaningful values
float ConvertToPercentage(int8_t score) {
    // For int8 quantized models, typically -128 to 127 maps to 0-1
    return (score + 128) / 255.0f * 100.0f;
}

void RespondToDetection(int detected_class, int8_t score) {
    // Initialize LED pin first time
    if (!is_initialized) {
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        is_initialized = true;
    }

    static int frame_counter = 0;
    frame_counter++;
    
    // Print frame counter every 30 frames
    if (frame_counter % 30 == 0) {
        MicroPrintf("Processing frame %d", frame_counter);
    }

    // Get the class label
    const char* sign_type = 
        (detected_class >= 0 && detected_class < kCategoryCount) ? 
        kCategoryLabels[detected_class] : "Unknown";
    
    // Convert score to percentage
    float confidence = ConvertToPercentage(score);

    // Debug low confidence detections
    if (confidence <= 70.0f) {
      MicroPrintf("Low confidence detection - Sign: %s, Confidence: %.1f%%", 
          (detected_class >= 0 && detected_class < kCategoryCount) ? 
          kCategoryLabels[detected_class] : "Unknown", 
          confidence);
  }

    // Only print if we have a high confidence detection or class changed
    if (confidence > 70.0f) {
        if (detected_class != last_detected_class) {
            // New sign detected!
            MicroPrintf("\n*** NEW SIGN DETECTED ***");
            MicroPrintf("Sign: %s", sign_type);
            MicroPrintf("Confidence: %.1f%%", confidence);
            MicroPrintf("Class ID: %d\n", detected_class);
            
            last_detected_class = detected_class;
            same_class_count = 1;
            
            // Flash LED for new detection
            for (int i = 0; i < 3; i++) {
                gpio_put(LED_PIN, 1);
                sleep_ms(50);
                gpio_put(LED_PIN, 0);
                sleep_ms(50);
            }
        } else {
            // Same sign, increment counter
            same_class_count++;
            
            // Print update every 10 detections of same sign
            if (same_class_count % 10 == 0) {
                MicroPrintf("Still detecting: %s (%.1f%% confidence, count: %d)", 
                           sign_type, confidence, same_class_count);
            }
            
            // Keep LED on for consistent detection
            gpio_put(LED_PIN, 1);
        }
    } else {
        // Low confidence - might be transitioning or no clear sign
        if (last_detected_class != -1 && confidence < 50.0f) {
            MicroPrintf("Lost detection of: %s", 
                       kCategoryLabels[last_detected_class]);
            last_detected_class = -1;
            same_class_count = 0;
        }
        
        // LED off for no/low confidence detection
        gpio_put(LED_PIN, 0);
    }
}