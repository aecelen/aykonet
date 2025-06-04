// #include "image_provider.h"
// #include "model_settings.h"
// #include "test_images.h"
// #include "pico/stdlib.h"
// #include "main_functions.h" // Include to access SetLastClassId function

// // Make image_index accessible to other files
// uint32_t image_index = 0;

// TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
  
//   // For now, we'll use the test images from test_images.h
//   // In a real application, you would capture images from a camera
  
//   // Instead of manually listing test images, we'll use the get_test_image_by_class
//   // function that's generated in test_images.h
  
//   // Get the current test image index (cycle through 0-42)
//   uint8_t current_class = image_index % NUM_TEST_IMAGES;
  
//   // Get the class ID for the current test image
//   uint8_t class_id = test_image_classes[current_class];
  
//   // Store the class ID for accuracy calculation
//   SetLastClassId(class_id);
  
//   // Get the corresponding test image data
//   const int8_t* current_image = get_test_image_by_class(class_id);
  
//   // Report which image/class we're using
//   // MicroPrintf("Class %d - %s",  
//   //             class_id, 
//   //             kCategoryLabels[class_id]);
  
//   // Make sure we have a valid image
//   if (current_image == NULL) {
//     MicroPrintf("ERROR: Test image for class %d not found!", class_id);
//     return kTfLiteError;
//   }
  
//   // Copy the test image data into the input tensor
//   for (int i = 0; i < image_width * image_height * channels; ++i) {
//     image_data[i] = current_image[i];
//   }
  
//   // Move to the next test image for the next inference
//   image_index++;
  
//   return kTfLiteOk;
// }
#include "image_provider.h"
#include "model_settings.h"
#include "pico/stdlib.h"
#include "main_functions.h"
#include "hm01b0.h"
#include <cstring>
#include <cmath>

// Camera configuration
static const struct hm01b0_config hm01b0_config = {
    .i2c           = i2c0,
    .sda_pin       = 4,
    .scl_pin       = 5,
    .vsync_pin     = 6,
    .hsync_pin     = 7,
    .pclk_pin      = 8,
    .data_pin_base = 9,
    .data_bits     = 1,
    .pio           = pio0,
    .pio_sm        = 0,
    .reset_pin     = -1,   // Not connected
    .mclk_pin      = -1,   // Not connected
    .width         = 160,
    .height        = 120,
};

// Buffer for camera image (160x120)
static uint8_t camera_buffer[160 * 120];
static bool camera_initialized = false;

// Initialize camera once
static bool init_camera() {
    if (camera_initialized) return true;
    
    if (hm01b0_init(&hm01b0_config) != 0) {
        MicroPrintf("Failed to initialize camera!");
        return false;
    }
    
    // Set exposure
    hm01b0_set_coarse_integration(650);
    
    camera_initialized = true;
    MicroPrintf("Camera initialized successfully");
    return true;
}

// Simple image downsampling from 160x120 to 32x32
static void downsample_image(const uint8_t* src, int8_t* dst) {
    // Calculate scaling factors
    const float x_scale = 160.0f / 32.0f;  // 5.0
    const float y_scale = 120.0f / 32.0f;  // 3.75
    
    // For each pixel in the destination image
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            // Calculate the corresponding region in the source image
            int src_x_start = (int)(x * x_scale);
            int src_y_start = (int)(y * y_scale);
            int src_x_end = (int)((x + 1) * x_scale);
            int src_y_end = (int)((y + 1) * y_scale);
            
            // Ensure we don't go out of bounds
            if (src_x_end > 160) src_x_end = 160;
            if (src_y_end > 120) src_y_end = 120;
            
            // Average all pixels in the region
            int sum = 0;
            int count = 0;
            
            for (int sy = src_y_start; sy < src_y_end; sy++) {
                for (int sx = src_x_start; sx < src_x_end; sx++) {
                    sum += src[sy * 160 + sx];
                    count++;
                }
            }
            
            // Calculate average and convert to int8
            // The model expects quantized int8 input with scale=0.003921568859368563, zero_point=-128
            // This means: real_value = (int8_value + 128) * 0.003921568859368563
            // So: int8_value = (real_value / 0.003921568859368563) - 128
            
            uint8_t avg = (count > 0) ? (sum / count) : 0;
            
            // Normalize to [0, 1] range first
            float normalized = avg / 255.0f;
            
            // Apply quantization
            int8_t quantized = (int8_t)(normalized / 0.003921568859368563f - 128);
            
            // Store in destination
            dst[y * 32 + x] = quantized;
        }
    }
}

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
    // Verify expected dimensions
    if (image_width != kNumCols || image_height != kNumRows || channels != kNumChannels) {
        MicroPrintf("ERROR: Unexpected image dimensions");
        return kTfLiteError;
    }
    
    // Initialize camera on first use
    if (!init_camera()) {
        MicroPrintf("ERROR: Camera initialization failed");
        return kTfLiteError;
    }
    
    // Capture image from camera
    MicroPrintf("Capturing image...");
    hm01b0_read_frame(camera_buffer, sizeof(camera_buffer));
    
    // Downsample and quantize the image
    downsample_image(camera_buffer, image_data);
    
    // Since we're using live camera, we don't know the true class
    // Set it to 255 (unknown) for now
    SetLastClassId(255);
    
    return kTfLiteOk;
}