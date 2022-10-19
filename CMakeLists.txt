cmake_minimum_required(VERSION 3.22)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_EXTRAS_SDK_PATH}/external/pico_extras_import.cmake)
include($ENV{PICO_SDK_PATH}/../RF24/CMakeLists.txt)

project(nrf-temperature C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()


# 1 Create an INTERFACE library aggregating all the common parts of the application
add_library(common INTERFACE)

# note cmake policy is to use absolute paths for interface libraries.
target_sources(common INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/pin/pin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sleep/sleep.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/radio/radio.cpp
)

add_subdirectory(modules/pico-onewire)

target_compile_definitions(common INTERFACE
    
)

# can include library dependencies here
target_link_libraries(common INTERFACE
    pico_stdlib
    hardware_sleep
    hardware_spi
    RF24
    pico_one_wire
)


# ----- measure-temperature -----
add_executable(measure_temperature)
target_link_libraries(measure_temperature common
    hardware_irq
    hardware_i2c
)
target_compile_definitions(measure_temperature PRIVATE
    BLINK_LED=false
    LOW_POWER=true
)
target_sources(measure_temperature PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/temperature/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/temperature/temperature.cpp
)
pico_add_extra_outputs(measure_temperature)
# Enable usb output, disable uart output
pico_enable_stdio_usb(measure_temperature 1)
pico_enable_stdio_uart(measure_temperature 0)

# ----- relay -----
add_executable(relay)
target_link_libraries(relay common
    pico_multicore
)
target_compile_definitions(relay PRIVATE
    BLINK_CORE=true
    HANDLE_RADIO_FAILURES=true
    LOW_POWER=false
)
target_sources(relay PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/relay/main.cpp
)
pico_add_extra_outputs(relay)
# Enable usb output, disable uart output
pico_enable_stdio_usb(relay 1)
pico_enable_stdio_uart(relay 0)
