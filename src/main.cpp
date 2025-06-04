// #include "main_functions.h"
// #include "pico/stdlib.h"
// #include "pico/time.h"
// #include <cstdio>
// #include <cstdarg>
// #include <cstdio>

// int main(int argc, char* argv[]) {
//     stdio_init_all();
//     sleep_ms(2000);

//     while (!stdio_usb_connected()) {
//         tight_loop_contents();
//     }

//     // Debug
//     const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
    
//     for (int i = 0; i < 5; i++) {
//         gpio_put(LED_PIN, 1);
//         sleep_ms(100);
//         gpio_put(LED_PIN, 0);
//         sleep_ms(100);
//     }

//     sleep_ms(1000); 
    
//     printf("AykoNet is running...\n\n");
    
//     setup();

//     // Run for exactly 43 iterations (one for each traffic sign class)
//     int iterations = 0;
//     const int MAX_ITERATIONS = 43;
    
    
//     while (true) {
//         loop();

//         iterations++;
//         if (iterations >= MAX_ITERATIONS) {
        
//             // Get performance metrics
//             int64_t avg_inference_time_us;
//             float accuracy;
//             GetPerformanceMetrics(&avg_inference_time_us, &accuracy);
            
//             printf("\n========== PERFORMANCE REPORT ==========\n");
//             printf("Inference time: %.2f ms \n", avg_inference_time_us / 1000.0f);
//             printf("Accuracy: %.2f%%\n", accuracy);
//             printf("========================================\n\n");
            
//             sleep_ms(30000);
//             iterations = 0;
//         }
//     }
    
//     return 0;
// }
#include "main_functions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    stdio_init_all();
    sleep_ms(2000);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    // Debug LED
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Blink to indicate startup
    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
    }

    sleep_ms(1000); 
    
    printf("Traffic Sign Recognition with Camera\n");
    printf("AykoNet is running...\n\n");
    
    setup();

    printf("Press 'c' to capture and classify an image\n");
    printf("Press 'a' for automatic continuous mode\n");
    printf("Press 's' to stop automatic mode\n");
    printf("Press 'h' for help\n\n");

    bool auto_mode = false;
    int capture_count = 0;
    
    while (true) {
        // Check for user input
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == 'c') {
                // Single capture
                printf("\nCapturing and classifying...\n");
                loop();
                capture_count++;
            }
            else if (c == 'a') {
                printf("\nStarting automatic mode. Press 's' to stop.\n");
                auto_mode = true;
            }
            else if (c == 's') {
                auto_mode = false;
                printf("\nStopped automatic mode.\n");
                
                // Show performance metrics if we've done some captures
                if (capture_count > 0) {
                    int64_t avg_inference_time_us;
                    float accuracy;
                    GetPerformanceMetrics(&avg_inference_time_us, &accuracy);
                    
                    printf("\n========== PERFORMANCE REPORT ==========\n");
                    printf("Total captures: %d\n", capture_count);
                    printf("Average inference time: %.2f ms\n", avg_inference_time_us / 1000.0f);
                    printf("========================================\n\n");
                }
            }
            else if (c == 'h') {
                printf("\n=== Help ===\n");
                printf("c - Capture and classify single image\n");
                printf("a - Start automatic continuous capture\n");
                printf("s - Stop automatic mode\n");
                printf("h - Show this help\n\n");
            }
        }
        
        // Automatic mode
        if (auto_mode) {
            loop();
            capture_count++;
            sleep_ms(500); // Capture every 500ms
        }
        
        sleep_ms(10);
    }
    
    return 0;
}