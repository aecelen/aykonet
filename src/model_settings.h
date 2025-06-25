#ifndef TRAFFIC_SIGN_MODEL_SETTINGS_H_
#define TRAFFIC_SIGN_MODEL_SETTINGS_H_

// Image input size
constexpr int kNumCols = 64;
constexpr int kNumRows = 64;
constexpr int kNumChannels = 1;  // Change to 3 if using RGB model

constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;

// Number of classes in your traffic sign model (assuming 43 classes)
constexpr int kCategoryCount = 43;

// Class labels for traffic signs
extern const char* kCategoryLabels[kCategoryCount];

#endif  // TRAFFIC_SIGN_MODEL_SETTINGS_H_