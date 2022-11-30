add_library(radio INTERFACE)

add_subdirectory(RF24)

target_sources(radio INTERFACE
    radio.cpp
)

target_link_libraries(radio INTERFACE
    pico_stdlib
    hardware_spi
    RF24
)

target_include_directories(radio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
