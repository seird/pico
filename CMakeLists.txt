cmake_minimum_required(VERSION 3.22)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_EXTRAS_SDK_PATH}/external/pico_extras_import.cmake)

project(nrf-temperature C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_library(common INTERFACE)
target_compile_definitions(common INTERFACE
    PAYLOAD_SIZE=5 # Header (1) + Data (4)
)

add_subdirectory(src)
