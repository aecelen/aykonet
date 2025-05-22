#include "image_provider.h"
#include "model_settings.h"
#include "pico/stdlib.h"
#include "main_functions.h"
#include <string.h>

// C linkage for HM01B0 functions
extern "C" {
#include "hm01b0.h"
}

// Camera configuration
const struct hm01b0_config camera_config = {
    .i2c           = i2c0,
    .sda_pin       = 4,
    .scl_pin       = 5,
    .vsync_pin     = 6,
    .hsync_pin     = 7,
    .pclk_pin      = 8,
    .data_pin_base = 9,
    .data_bits     = 1,        // Use 1-bit mode for Raspberry Pi Pico
    .pio           = pio0,
    .pio_sm        = 0,
    .reset_pin     = -1,       // Not connected
    .mclk_pin      = -1,       // Not connected
    .width         = 160,      // Minimum resolution
    .height        = 120,
};

// Camera frame buffer
static uint8_t camera_buffer[160 * 120];
static bool camera_initialized = false;

// Simple bilinear interpolation for image resizing
static void resize_image(const uint8_t* src, int src_width, int src_height,
                        uint8_t* dst, int dst_width, int dst_height) {
    float x_ratio = (float)src_width / dst_width;
    float y_ratio = (float)src_height / dst_height;
    
    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;
            
            int x1 = (int)src_x;
            int y1 = (int)src_y;
            int x2 = x1 + 1;
            int y2 = y1 + 1;
            
            // Boundary checks
            if (x2 >= src_width) x2 = src_width - 1;
            if (y2 >= src_height) y2 = src_height - 1;
            
            float dx = src_x - x1;
            float dy = src_y - y1;
            
            // Get the four neighboring pixels
            uint8_t p11 = src[y1 * src_width + x1];
            uint8_t p12 = src[y2 * src_width + x1];
            uint8_t p21 = src[y1 * src_width + x2];
            uint8_t p22 = src[y2 * src_width + x2];
            
            // Bilinear interpolation
            float interpolated = p11 * (1 - dx) * (1 - dy) +
                               p21 * dx * (1 - dy) +
                               p12 * (1 - dx) * dy +
                               p22 * dx * dy;
            
            dst[y * dst_width + x] = (uint8_t)interpolated;
        }
    }
}

// Convert uint8 to int8 with quantization
static void convert_to_int8(const uint8_t* src, int8_t* dst, int size) {
    // Convert from uint8 [0,255] to int8 [-128,127]
    // Apply quantization: scale=0.003921568859368563, zero_point=-128
    // This matches the quantization parameters from your model
    for (int i = 0; i < size; i++) {
        // Normalize to [0,1], then apply quantization
        float normalized = src[i] / 255.0f;
        int quantized = (int)(normalized / 0.003921568859368563) - 128;
        
        // Clamp to int8 range
        if (quantized > 127) quantized = 127;
        if (quantized < -128) quantized = -128;
        
        dst[i] = (int8_t)quantized;
    }
}

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
    // Initialize camera on first call
    if (!camera_initialized) {
        MicroPrintf("Initializing camera...");
        
        if (hm01b0_init(&camera_config) != 0) {
            MicroPrintf("ERROR: Failed to initialize camera!");
            return kTfLiteError;
        }
        
        // Optional: Set exposure (integration time)
        // Lower values = less exposure, higher values = more exposure
        // Range: 2 to 0xFFFF lines
        hm01b0_set_coarse_integration(100);  // Adjust as needed
        
        camera_initialized = true;
        MicroPrintf("Camera initialized successfully");
        
        // Give camera some time to stabilize
        sleep_ms(1000);
    }
    
    // Capture frame from camera
    MicroPrintf("Capturing frame...");
    hm01b0_read_frame(camera_buffer, sizeof(camera_buffer));
    
    // Check if we need to resize the image
    if (image_width == 160 && image_height == 120 && channels == 1) {
        // Direct copy if dimensions match
        convert_to_int8(camera_buffer, image_data, image_width * image_height);
    } else if (image_width == 32 && image_height == 32 && channels == 1) {
        // Resize from 160x120 to 32x32
        uint8_t resized_buffer[32 * 32];
        resize_image(camera_buffer, 160, 120, resized_buffer, 32, 32);
        convert_to_int8(resized_buffer, image_data, 32 * 32);
    } else {
        MicroPrintf("ERROR: Unsupported image dimensions: %dx%dx%d", 
                   image_width, image_height, channels);
        return kTfLiteError;
    }
    
    MicroPrintf("Frame captured and processed");
    
    // Since we're using real camera data, we don't know the true class
    // Set a dummy class ID (or remove accuracy tracking for camera mode)
    SetLastClassId(0);  // Unknown class
    
    return kTfLiteOk;
}