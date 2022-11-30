add_library(pin INTERFACE)

target_sources(pin INTERFACE
    pin.cpp
)

target_link_libraries(pin INTERFACE
    pico_stdlib
)

target_include_directories(pin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
