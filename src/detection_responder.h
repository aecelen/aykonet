#ifndef TRAFFIC_SIGN_DETECTION_RESPONDER_H_
#define TRAFFIC_SIGN_DETECTION_RESPONDER_H_

#include "pico-tflmicro/src/tensorflow/lite/c/common.h"
#include "pico-tflmicro/src/tensorflow/lite/micro/micro_log.h"

// Called when a traffic sign is detected with its class and confidence
void RespondToDetection(int detected_class, int8_t score);

#endif  // TRAFFIC_SIGN_DETECTION_RESPONDER_H_