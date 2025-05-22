#ifndef TRAFFIC_SIGN_IMAGE_PROVIDER_H_
#define TRAFFIC_SIGN_IMAGE_PROVIDER_H_

#include "pico-tflmicro/src/tensorflow/lite/c/common.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_log.h"

// This gets an image from a source like a camera, or from test images
// For testing, we'll use the test images from test_images.h
TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data);

#endif  // TRAFFIC_SIGN_IMAGE_PROVIDER_H_