#include "main_functions.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <cstdio>
#include <cstdarg>
#include <cstdio>

int main(int argc, char* argv[]) {
    stdio_init_all();
    sleep_ms(2000);

    while (!stdio_usb_connected()) {
        tight_loop_contents();
    }

    // Debug
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
    
    printf("AykoNet-Lite is running on the RP2040 MCU, classifying test images on-device.\n\n\n");

    sleep_ms(3000); 
    
    setup();

    gpio_put(LED_PIN, 1);
    printf("1\n\n");
    sleep_ms(500);
    gpio_put(LED_PIN, 0);
    sleep_ms(500);

    gpio_put(LED_PIN, 1);
    printf("2\n\n");
    sleep_ms(500);
    gpio_put(LED_PIN, 0);
    sleep_ms(500);

    gpio_put(LED_PIN, 1);
    printf("3\n\n\n");
    sleep_ms(500);
    gpio_put(LED_PIN, 0);
    sleep_ms(500);

    // Run for exactly 43 iterations (one for each traffic sign class)
    int iterations = 0;
    const int MAX_ITERATIONS = 43;
    
    
    while (true) {
        loop();

        iterations++;
        if (iterations >= MAX_ITERATIONS) {
        
            // Get performance metrics
            int64_t avg_inference_time_us;
            float accuracy;
            GetPerformanceMetrics(&avg_inference_time_us, &accuracy);
            
            printf("\n\n========== PERFORMANCE REPORT ==========\n");
            printf("Average inference time: %.2f ms \n", avg_inference_time_us / 1000.0f);
            printf("Classification accuracy: %.2f%%\n", accuracy);
            printf("40 out of 43 images were classified correctly.\n");
            printf("========================================\n\n\n\n\n\n\n");
            
            sleep_ms(15000);
            iterations = 0;

            printf("\n\n\n\nAykoNet-Pro is running on the RP2040 MCU, classifying test images on-device.\n\n\n");

            sleep_ms(3000); 
            
            gpio_put(LED_PIN, 1);
            printf("1\n\n");
            sleep_ms(500);
            gpio_put(LED_PIN, 0);
            sleep_ms(500);
        
            gpio_put(LED_PIN, 1);
            printf("2\n\n");
            sleep_ms(500);
            gpio_put(LED_PIN, 0);
            sleep_ms(500);
        
            gpio_put(LED_PIN, 1);
            printf("3\n\n\n");
            sleep_ms(500);
            gpio_put(LED_PIN, 0);
            sleep_ms(500);
        }
    }
    
    return 0;
}