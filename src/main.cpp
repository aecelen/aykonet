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

    // Debug LED initialization
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // LED blink to indicate startup
    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
    }

    sleep_ms(1000); 
    
    printf("AykoNet Traffic Sign Recognition with Camera\n");
    printf("Initializing...\n\n");
    
    setup();
    
    printf("Starting continuous inference...\n");
    printf("Press Ctrl+C to view performance metrics\n\n");
    
    int inference_count = 0;
    const int REPORT_INTERVAL = 20; // Report every 10 inferences
    
    while (true) {
        loop();
        
        inference_count++;
        
        // Periodically report performance
        if (inference_count % REPORT_INTERVAL == 0) {
            int64_t avg_inference_time_us;
            int total_inferences;
            GetPerformanceMetrics(&avg_inference_time_us, &total_inferences);
            printf("\n\n");
            printf("========== PERFORMANCE REPORT ==========\n");
            printf("Total inferences: %d\n", total_inferences);
            printf("Average inference time: %.2f ms\n", avg_inference_time_us / 1000.0f);
            printf("========================================\n\n");
            printf("\n");
        }
        
        // Small delay to prevent overwhelming the system
        sleep_ms(50);
    }
    
    return 0;
}