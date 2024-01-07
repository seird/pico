add_library(mpu6000 INTERFACE)

target_sources(mpu6000 INTERFACE
    mpu6000.c
)

target_link_libraries(mpu6000 INTERFACE
    pico_stdlib hardware_i2c
)

target_include_directories(mpu6000 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
