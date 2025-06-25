#ifndef TRAFFIC_SIGN_MAIN_FUNCTIONS_H_
#define TRAFFIC_SIGN_MAIN_FUNCTIONS_H_

#include <stdint.h> // For int64_t type

// Expose a C friendly interface for main functions.
#ifdef __cplusplus
extern "C" {
#endif

// Initializes all data needed for the example. The name is important, and needs
// to be setup() for Arduino compatibility.
void setup();

// Runs one iteration of data gathering and inference. This should be called
// repeatedly from the application code. The name needs to be loop() for Arduino
// compatibility.
void loop();

// Get performance metrics after all iterations
void GetPerformanceMetrics(int64_t* avg_time_us, int* total_inferences);

#ifdef __cplusplus
}
#endif

#endif  // TRAFFIC_SIGN_MAIN_FUNCTIONS_H_