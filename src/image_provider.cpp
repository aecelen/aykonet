#include "image_provider.h"
#include "model_settings.h"
#include "test_images.h"
#include "pico/stdlib.h"
#include "main_functions.h" // Include to access SetLastClassId function

// Make image_index accessible to other files
uint32_t image_index = 0;

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
  
  // For now, we'll use the test images from test_images.h
  // In a real application, you would capture images from a camera
  
  // Instead of manually listing test images, we'll use the get_test_image_by_class
  // function that's generated in test_images.h
  
  // Get the current test image index (cycle through 0-42)
  uint8_t current_class = image_index % NUM_TEST_IMAGES;
  
  // Get the class ID for the current test image
  uint8_t class_id = test_image_classes[current_class];
  
  // Store the class ID for accuracy calculation
  SetLastClassId(class_id);
  
  // Get the corresponding test image data
  const int8_t* current_image = get_test_image_by_class(class_id);
  
  // Report which image/class we're using
  MicroPrintf("Class %d - %s",  
              class_id, 
              kCategoryLabels[class_id]);
  
  // Make sure we have a valid image
  if (current_image == NULL) {
    MicroPrintf("ERROR: Test image for class %d not found!", class_id);
    return kTfLiteError;
  }
  
  // Copy the test image data into the input tensor
  for (int i = 0; i < image_width * image_height * channels; ++i) {
    image_data[i] = current_image[i];
  }
  
  // Move to the next test image for the next inference
  image_index++;
  
  return kTfLiteOk;
}