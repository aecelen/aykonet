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
    
    printf("AykoNet is running...\n\n");
    
    setup();

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
            
            printf("\n========== PERFORMANCE REPORT ==========\n");
            printf("Inference time: %.2f ms \n", avg_inference_time_us / 1000.0f);
            printf("Accuracy: %.2f%%\n", accuracy);
            printf("========================================\n\n");
            
            sleep_ms(30000);
            iterations = 0;
        }
    }
    
    return 0;
}