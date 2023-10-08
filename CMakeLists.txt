cmake_minimum_required(VERSION 3.22)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_EXTRAS_SDK_PATH}/external/pico_extras_import.cmake)

project(pico-projects C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_library(common INTERFACE)
target_compile_definitions(common INTERFACE
)

add_subdirectory(src)
