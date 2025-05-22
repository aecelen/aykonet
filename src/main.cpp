#include "main_functions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    // Initialize stdio and wait for USB connection
    stdio_init_all();
    sleep_ms(4000);

    // LED initialization for debugging
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Blink LED to indicate startup
    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(200);
        gpio_put(LED_PIN, 0);
        sleep_ms(200);
    }

    printf("\n========================================\n");
    printf("AykoNet Traffic Sign Recognition\n");
    printf("With HM01B0 Camera Integration\n");
    printf("========================================\n\n");
    
    // Initialize the model and camera
    setup();
    
    printf("System ready. Starting continuous inference...\n\n");
    
    // Continuous operation mode
    int iterations = 0;
    const int REPORT_INTERVAL = 10;  // Report performance every 10 inferences
    
    while (true) {
        // Run one inference cycle
        loop();
        
        iterations++;
        
        // Periodic performance reporting
        if (iterations % REPORT_INTERVAL == 0) {
            int64_t avg_inference_time_us;
            float accuracy;
            GetPerformanceMetrics(&avg_inference_time_us, &accuracy);
            
            printf("\n--- Performance Report (after %d inferences) ---\n", iterations);
            printf("Average inference time: %.2f ms\n", avg_inference_time_us / 1000.0f);
            printf("Total inferences: %d\n", iterations);
            printf("---------------------------------------------\n\n");
        }
        
        // Keep LED on to indicate system is running
        gpio_put(LED_PIN, iterations % 2);
    }
    
    return 0;
}