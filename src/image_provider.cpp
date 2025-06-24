#include "image_provider.h"
#include "model_settings.h"
#include "pico/stdlib.h"
#include "hm01b0.h"
#include "main_functions.h"
#include <cstring>
#include <cstdio>

// Camera configuration
static const struct hm01b0_config camera_config = {
    .i2c           = i2c0,
    .sda_pin       = 4,
    .scl_pin       = 5,
    .vsync_pin     = 6,
    .hsync_pin     = 7,
    .pclk_pin      = 8,
    .data_pin_base = 9,
    .data_bits     = 8,    // Using 8-bit mode for better quality
    .pio           = pio0,
    .pio_sm        = 0,
    .reset_pin     = -1,   // Not connected
    .mclk_pin      = -1,   // Not connected
    .width         = 160,  // Camera native resolution
    .height        = 120,
};

// Camera frame buffer
static uint8_t camera_buffer[160 * 120];
static bool camera_initialized = false;

// Simple bilinear interpolation for image resizing
static void resize_image(const uint8_t* src, int src_width, int src_height,
                        int8_t* dst, int dst_width, int dst_height) {
    float x_ratio = (float)src_width / dst_width;
    float y_ratio = (float)src_height / dst_height;
    
    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {
            // Calculate source coordinates
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;
            
            // Get integer parts
            int x0 = (int)src_x;
            int y0 = (int)src_y;
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            
            // Clamp to image bounds
            if (x1 >= src_width) x1 = src_width - 1;
            if (y1 >= src_height) y1 = src_height - 1;
            
            // Get fractional parts
            float fx = src_x - x0;
            float fy = src_y - y0;
            
            // Get pixel values
            uint8_t p00 = src[y0 * src_width + x0];
            uint8_t p10 = src[y0 * src_width + x1];
            uint8_t p01 = src[y1 * src_width + x0];
            uint8_t p11 = src[y1 * src_width + x1];
            
            // Bilinear interpolation
            float val = (1 - fx) * (1 - fy) * p00 +
                        fx * (1 - fy) * p10 +
                        (1 - fx) * fy * p01 +
                        fx * fy * p11;
            
            // Convert to int8 with quantization
            // Model expects int8 with scale=0.003921568859368563, zero_point=-128
            // This means: quantized = (float_val / scale) + zero_point
            // For normalized [0,1] input: quantized = (val/255.0) / 0.00392... - 128
            float normalized = val / 255.0f;
            int quantized = (int)(normalized / 0.003921568859368563f) - 128;
            
            // Clamp to int8 range
            if (quantized < -128) quantized = -128;
            if (quantized > 127) quantized = 127;
            
            dst[y * dst_width + x] = (int8_t)quantized;
        }
    }
}

// Center crop the image to a square before resizing
static void center_crop_and_resize(const uint8_t* src, int src_width, int src_height,
                                  int8_t* dst, int dst_size) {
    // Calculate square crop dimensions
    int crop_size = (src_width < src_height) ? src_width : src_height;
    int x_offset = (src_width - crop_size) / 2;
    int y_offset = (src_height - crop_size) / 2;
    
    // Create temporary buffer for cropped image
    uint8_t* cropped = new uint8_t[crop_size * crop_size];
    
    // Copy center square from source
    for (int y = 0; y < crop_size; y++) {
        for (int x = 0; x < crop_size; x++) {
            int src_idx = (y + y_offset) * src_width + (x + x_offset);
            int dst_idx = y * crop_size + x;
            cropped[dst_idx] = src[src_idx];
        }
    }
    
    // Resize the cropped square to model input size
    resize_image(cropped, crop_size, crop_size, dst, dst_size, dst_size);
    
    delete[] cropped;
}

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
    // Initialize camera on first call
    if (!camera_initialized) {
        MicroPrintf("Initializing camera...");
        
        if (hm01b0_init(&camera_config) != 0) {
            MicroPrintf("ERROR: Failed to initialize camera!");
            return kTfLiteError;
        }
        
        // Set exposure (adjust as needed for your lighting conditions)
        hm01b0_set_coarse_integration(650);
        
        camera_initialized = true;
        MicroPrintf("Camera initialized successfully!");
        
        // Wait a bit for camera to stabilize
        sleep_ms(500);
    }
    
    // Capture frame from camera
    hm01b0_read_frame(camera_buffer, sizeof(camera_buffer));
    
    // The camera provides 160x120 grayscale image
    // We need to resize it to 32x32 for the model
    // Using center crop to maintain aspect ratio and focus on center
    center_crop_and_resize(camera_buffer, camera_config.width, camera_config.height,
                          image_data, image_width);
    
    // For real deployment, we don't have ground truth
    // Set a dummy class ID (you could remove this in production)
    SetLastClassId(255); // Invalid class to indicate camera input
    
    return kTfLiteOk;
}

// Optional: Function to capture and save raw camera frame for debugging
void CaptureRawFrame(uint8_t* buffer) {
    if (camera_initialized) {
        hm01b0_read_frame(buffer, camera_config.width * camera_config.height);
    }
}