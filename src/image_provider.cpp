#include "image_provider.h"
#include "model_settings.h"
#include "hm01b0.h"
#include "pico/stdlib.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_log.h"
#include <cmath>  // For roundf function
#include <cstring> // For memcpy function

// Camera configuration - Choose based on your hardware setup
static const struct hm01b0_config camera_config = {
    .i2c           = i2c0,        // Use i2c0 instance
    .sda_pin       = 4,           // GPIO 4 for I2C SDA
    .scl_pin       = 5,           // GPIO 5 for I2C SCL

    // Pin configuration - ADJUST THESE TO MATCH YOUR WIRING
    .vsync_pin     = 6,           // GPIO 6 for VSYNC
    .hsync_pin     = 7,           // GPIO 7 for HSYNC  
    .pclk_pin      = 8,           // GPIO 8 for pixel clock
    .data_pin_base = 9,           // GPIO 9 for data (only 1 bit used)
    .data_bits     = 1,           // Use 1-bit data mode (grayscale)
    .pio           = pio0,        // Use PIO 0
    .pio_sm        = 0,           // Use state machine 0
    .reset_pin     = -1,          // No reset pin (-1 = not connected)
    .mclk_pin      = -1,          // No master clock pin (-1 = not connected)

    // Use smallest available resolution to minimize processing
    .width         = 160,         // Minimum width supported
    .height        = 120,         // Minimum height supported
};

// Camera frame buffer
static uint8_t camera_buffer[160 * 120];
static bool camera_initialized = false;

const float INPUT_SCALE = 0.003921569f;
const int INPUT_ZERO_POINT = -128;

void resize_image_bilinear(const uint8_t* src, int src_width, int src_height,
                           uint8_t* dst, int dst_width, int dst_height) {
    float x_ratio = (float)(src_width - 1) / (dst_width - 1);
    float y_ratio = (float)(src_height - 1) / (dst_height - 1);
    
    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;
            
            int x1 = (int)src_x;
            int y1 = (int)src_y;
            int x2 = (x1 < src_width - 1) ? x1 + 1 : x1;
            int y2 = (y1 < src_height - 1) ? y1 + 1 : y1;
            
            float fx = src_x - x1;
            float fy = src_y - y1;
            
            uint8_t p1 = src[y1 * src_width + x1];
            uint8_t p2 = src[y1 * src_width + x2];
            uint8_t p3 = src[y2 * src_width + x1];
            uint8_t p4 = src[y2 * src_width + x2];
            
            float top = p1 * (1 - fx) + p2 * fx;
            float bottom = p3 * (1 - fx) + p4 * fx;
            float result = top * (1 - fy) + bottom * fy;
            
            dst[y * dst_width + x] = (uint8_t)roundf(result);
        }
    }
}

void quantize_normalized(const uint8_t* src, int8_t* dst, int size) {
    for (int i = 0; i < size; i++) {
        // Step 1: Normalize to [0, 1] like in training
        float normalized = src[i] / 255.0f;
        
        // Step 2: Apply model's quantization parameters
        float quantized = normalized / INPUT_SCALE + INPUT_ZERO_POINT;
        
        // Step 3: Clip to int8 range
        if (quantized < -128) quantized = -128;
        if (quantized > 127) quantized = 127;
        
        dst[i] = (int8_t)roundf(quantized);
    }
}

TfLiteStatus InitCamera() {
    if (camera_initialized) {
        return kTfLiteOk;
    }
    
    MicroPrintf("Initializing camera...\n");
    
    if (hm01b0_init(&camera_config) != 0) {
        MicroPrintf("Failed to initialize camera!");
        return kTfLiteError;
    }
    
    hm01b0_set_coarse_integration(400);
    
    camera_initialized = true;
    // MicroPrintf("Camera initialized with exposure = 400");
    
    return kTfLiteOk;
}

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
    // Make sure camera is initialized
    if (!camera_initialized) {
        if (InitCamera() != kTfLiteOk) {
            return kTfLiteError;
        }
    }
    
    // Capture frame from camera
    hm01b0_read_frame(camera_buffer, sizeof(camera_buffer));
    
    // Create temporary buffer for resized image
    uint8_t resized_buffer[kNumCols * kNumRows];
    
    resize_image_bilinear(camera_buffer, 160, 120, 
        resized_buffer, 32, 32);

    quantize_normalized(resized_buffer, image_data, 32 * 32);
    
    // MicroPrintf("Image captured and preprocessed");
    
    return kTfLiteOk;
}