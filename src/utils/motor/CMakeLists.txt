add_library(motor INTERFACE)

target_sources(motor INTERFACE
    motor.c
)

target_link_libraries(motor INTERFACE
    pico_stdlib hardware_pwm
)

target_include_directories(motor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
