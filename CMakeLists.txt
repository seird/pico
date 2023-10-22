cmake_minimum_required(VERSION 3.22)


if (NOT PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
        message("Setting PICO_SDK_PATH from environment ('${PICO_SDK_PATH}')")
    else()
        set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
        message("Setting PICO_SDK_PATH from git submodule path ('${PICO_SDK_PATH}')")
    endif()
else()
    message("PICO_SDK_PATH is ('${PICO_SDK_PATH}')")
endif()


if (NOT PICO_EXTRAS_PATH)
    if(DEFINED ENV{PICO_EXTRAS_PATH})
        set(PICO_EXTRAS_PATH $ENV{PICO_EXTRAS_PATH})
        message("Setting PICO_EXTRAS_PATH from environment ('${PICO_EXTRAS_PATH}')")
    else()
        set(PICO_EXTRAS_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-extras)
        message("Setting PICO_EXTRAS_PATH from git submodule path ('${PICO_EXTRAS_PATH}')")
    endif()
else()
    message("PICO_EXTRAS_PATH is ('${PICO_EXTRAS_PATH}')")
endif()


include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(pico-projects C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_library(common INTERFACE)
target_compile_definitions(common INTERFACE
)

add_subdirectory(src)
