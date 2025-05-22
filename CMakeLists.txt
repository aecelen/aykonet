cmake_minimum_required(VERSION 3.13)

# Initialize the Pico SDK
include(pico_sdk_import.cmake)

project(traffic_sign_recognition C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Initialize the Pico SDK
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pico-tflmicro)

# Add the executable
add_executable(traffic_sign_recognition
    src/main.cpp
    src/main_functions.cpp
    src/image_provider.cpp
    src/detection_responder.cpp
    src/model_settings.cpp
    model/traffic_sign_model_data.cc
    # Include HM01B0 source directly
    ../hm01b0-library-for-pico/src/hm01b0.c
)

# Include paths
target_include_directories(traffic_sign_recognition PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/model
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../hm01b0-library-for-pico/src/include
)

# Compiler flags - separate for C and C++
target_compile_options(traffic_sign_recognition PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    -O3
)

# Link libraries
target_link_libraries(traffic_sign_recognition
    pico_stdlib
    pico-tflmicro
    hardware_gpio
    hardware_pwm
    hardware_i2c
    hardware_pio
    hardware_dma
    cmsis_core
)

# Enable USB output, disable UART
pico_enable_stdio_usb(traffic_sign_recognition 1)
pico_enable_stdio_uart(traffic_sign_recognition 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(traffic_sign_recognition)