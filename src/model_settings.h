#ifndef TRAFFIC_SIGN_MODEL_SETTINGS_H_
#define TRAFFIC_SIGN_MODEL_SETTINGS_H_

// Image input size
constexpr int kNumCols = 32;
constexpr int kNumRows = 32;
constexpr int kNumChannels = 3;  // Changed from 1 to 3 for RGB (MobileNetV1_20)

constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;

// Number of classes in your traffic sign model (assuming 43 classes)
constexpr int kCategoryCount = 43;

// Class labels for traffic signs
extern const char* kCategoryLabels[kCategoryCount];

#endif  // TRAFFIC_SIGN_MODEL_SETTINGS_H_