#include "image_provider.h"
#include "model_settings.h"
#include "hm01b0.h"
#include "pico/stdlib.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_log.h"
#include <cmath>  // For roundf function

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

// // Simple nearest neighbor resize (faster than bilinear)
// void resize_image(const uint8_t* src, int src_width, int src_height, 
//                   uint8_t* dst, int dst_width, int dst_height) {
//     // For 160x120 to 32x32: every 5x3.75 pixels becomes 1 pixel
//     // We'll sample every 5th pixel in x and every 3.75th pixel in y
//     float x_ratio = (float)src_width / dst_width;   // 160/32 = 5.0
//     float y_ratio = (float)src_height / dst_height; // 120/32 = 3.75
    
//     for (int y = 0; y < dst_height; y++) {
//         for (int x = 0; x < dst_width; x++) {
//             int src_x = (int)(x * x_ratio);
//             int src_y = (int)(y * y_ratio);
//             dst[y * dst_width + x] = src[src_y * src_width + src_x];
//         }
//     }
// }

// // Simple bilinear interpolation resize function
// void resize_image(const uint8_t* src, int src_width, int src_height, 
//                     uint8_t* dst, int dst_width, int dst_height) {
//     float x_ratio = (float)src_width / dst_width;
//     float y_ratio = (float)src_height / dst_height;

//     for (int y = 0; y < dst_height; y++) {
//         for (int x = 0; x < dst_width; x++) {
//             // Calculate source coordinates
//             float src_x = x * x_ratio;
//             float src_y = y * y_ratio;

//             // Get integer coordinates
//             int x1 = (int)src_x;
//             int y1 = (int)src_y;
//             int x2 = (x1 + 1 < src_width) ? x1 + 1 : x1;
//             int y2 = (y1 + 1 < src_height) ? y1 + 1 : y1;

//             // Get fractional parts
//             float fx = src_x - x1;
//             float fy = src_y - y1;

//             // Get pixel values
//             uint8_t p1 = src[y1 * src_width + x1];  // Top-left
//             uint8_t p2 = src[y1 * src_width + x2];  // Top-right
//             uint8_t p3 = src[y2 * src_width + x1];  // Bottom-left
//             uint8_t p4 = src[y2 * src_width + x2];  // Bottom-right

//             // Bilinear interpolation
//             float top = p1 * (1 - fx) + p2 * fx;
//             float bottom = p3 * (1 - fx) + p4 * fx;
//             float result = top * (1 - fy) + bottom * fy;

//             dst[y * dst_width + x] = (uint8_t)result;
//         }
//     }
// }

// void resize_image(const uint8_t* src, int src_width, int src_height,
//                             uint8_t* dst, int dst_width, int dst_height) {
//     // Average pixels in each block (best for downsampling)
//     // For 160x120 -> 32x32: each output pixel averages a 5x3.75 block

//     float x_ratio = (float)src_width / dst_width;   // 5.0
//     float y_ratio = (float)src_height / dst_height; // 3.75

//     for (int y = 0; y < dst_height; y++) {
//         for (int x = 0; x < dst_width; x++) {
//         // Calculate source block boundaries
//         int x_start = (int)(x * x_ratio);
//         int x_end = (int)((x + 1) * x_ratio);
//         int y_start = (int)(y * y_ratio);
//         int y_end = (int)((y + 1) * y_ratio);

//         // Clamp to image boundaries
//         if (x_end > src_width) x_end = src_width;
//         if (y_end > src_height) y_end = src_height;

//         // Average all pixels in the block
//         int sum = 0;
//         int count = 0;
//         for (int sy = y_start; sy < y_end; sy++) {
//             for (int sx = x_start; sx < x_end; sx++) {
//                 sum += src[sy * src_width + sx];
//                 count++;
//             }
//         }

//         dst[y * dst_width + x] = (count > 0) ? (sum / count) : 0;
//         }
//     }
// }

// Area averaging resize optimized for 160x120 -> 64x64
void resize_image(const uint8_t* src, int src_width, int src_height, 
                    uint8_t* dst, int dst_width, int dst_height) {
    // For 160x120 -> 64x64: each output pixel averages a 2.5x1.875 block
    // This gives better quality than simple decimation for 64x64

    float x_ratio = (float)src_width / dst_width;   // 160/64 = 2.5
    float y_ratio = (float)src_height / dst_height; // 120/64 = 1.875

    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {
            // Calculate source block boundaries
            int x_start = (int)(x * x_ratio);
            int x_end = (int)((x + 1) * x_ratio);
            int y_start = (int)(y * y_ratio);
            int y_end = (int)((y + 1) * y_ratio);

            // Clamp to image boundaries
            if (x_end > src_width) x_end = src_width;
            if (y_end > src_height) y_end = src_height;

            // Average all pixels in the block
            int sum = 0;
            int count = 0;
            for (int sy = y_start; sy < y_end; sy++) {
                for (int sx = x_start; sx < x_end; sx++) {
                    sum += src[sy * src_width + sx];
                    count++;
                }
            }

            dst[y * dst_width + x] = (count > 0) ? (sum / count) : 0;
        }
    }
}

// Quantize image using EXACT same parameters as your test images
void quantize_image(const uint8_t* src, int8_t* dst, int size) {
    // Your test image quantization parameters:
    // scale = 0.003921568859368563 (which is 1/255)
    // zero_point = -128
    // Formula: quantized = (normalized / scale) + zero_point
    
    const float scale = 0.003921568859368563f;  // Exact same as your test images
    const int zero_point = -128;                // Exact same as your test images
    
    for (int i = 0; i < size; i++) {
        // Step 1: Normalize camera pixel [0,255] to [0,1] (same as your training)
        float normalized = src[i] / 255.0f;
        
        // Step 2: Apply same quantization as your test images
        float quantized_float = (normalized / scale) + zero_point;
        
        // Step 3: Clamp to int8 range and convert
        int quantized_int = (int)roundf(quantized_float);
        if (quantized_int < -128) quantized_int = -128;
        if (quantized_int > 127) quantized_int = 127;
        
        dst[i] = (int8_t)quantized_int;
    }
    
    // Note: Since scale = 1/255, this simplifies to: dst[i] = src[i] - 128
    // But using the exact formula ensures perfect matching with your test images
}

TfLiteStatus InitCamera() {
    if (camera_initialized) {
        return kTfLiteOk;
    }
    
    MicroPrintf("Initializing camera...");
    
    if (hm01b0_init(&camera_config) != 0) {
        MicroPrintf("Failed to initialize camera!");
        return kTfLiteError;
    }
    
    // Set exposure (integration time) - IMPORTANT for image quality
    // Range: 2 to 65535 lines
    // Lower values = darker images, less motion blur
    // Higher values = brighter images, more motion blur
    // For traffic signs (usually bright/reflective), moderate exposure works well
    hm01b0_set_coarse_integration(450);  // Start with moderate exposure - 300
    
    camera_initialized = true;
    MicroPrintf("Camera initialized with exposure = 100");
    
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
    
    // Resize from 160x120 to 32x32
    resize_image(camera_buffer, 160, 120, 
                 resized_buffer, kNumCols, kNumRows);
    
    // Quantize for model input
    quantize_image(resized_buffer, image_data, kNumCols * kNumRows * channels);
    
    // MicroPrintf("Image captured and preprocessed");
    
    return kTfLiteOk;
}