#include "main_functions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cstdio>
#include <cstdarg>

// Performance tracking
static int inference_count = 0;
static int64_t total_time_us = 0;
static absolute_time_t last_report_time;

int main(int argc, char* argv[]) {
    stdio_init_all();
    sleep_ms(2000);

    // Wait for USB connection
    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    // Debug LED blink
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    for (int i = 0; i < 5; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
    }

    sleep_ms(1000); 
    
    printf("\n===========================================\n");
    printf("AykoNet Real-time Traffic Sign Recognition\n");
    printf("Camera Mode - Live Detection\n");
    printf("===========================================\n\n");
    
    setup();
    
    last_report_time = get_absolute_time();
    
    // Continuous real-time detection loop
    while (true) {
        absolute_time_t start_time = get_absolute_time();
        
        // Run inference
        loop();
        
        absolute_time_t end_time = get_absolute_time();
        int64_t inference_time_us = absolute_time_diff_us(start_time, end_time);
        
        total_time_us += inference_time_us;
        inference_count++;
        
        // Report performance every 5 seconds
        if (absolute_time_diff_us(last_report_time, end_time) > 5000000) { // 5 seconds
            float avg_inference_ms = (float)total_time_us / inference_count / 1000.0f;
            float fps = 1000000.0f / ((float)total_time_us / inference_count);
            
            printf("\n--- Performance Report ---\n");
            printf("Inferences: %d\n", inference_count);
            printf("Avg inference time: %.2f ms\n", avg_inference_ms);
            printf("FPS: %.2f\n", fps);
            printf("-------------------------\n\n");
            
            // Reset counters
            inference_count = 0;
            total_time_us = 0;
            last_report_time = end_time;
        }
        
        // Small delay to control frame rate (optional)
        // Remove or adjust this for maximum FPS
        sleep_ms(50); // ~20 FPS max
    }
    
    return 0;
}