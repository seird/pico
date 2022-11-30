add_library(sleep INTERFACE)

target_sources(sleep INTERFACE
    sleep.cpp
)

target_link_libraries(sleep INTERFACE
    pico_stdlib
    hardware_sleep
)

target_include_directories(sleep INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
