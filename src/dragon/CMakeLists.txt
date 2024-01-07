# -----------------------------------------------
# ------------------- dragon --------------------
# -----------------------------------------------
add_executable(dragon)


target_link_libraries(dragon pin mpu6000 motor pid radio
    pico_stdlib pico_multicore tinyusb_device tinyusb_board
)
target_compile_definitions(dragon PRIVATE
    USE_CDC=false
    BLINK_LED=false
    LOW_POWER=false
    SYSTEM_FREQUENCY_KHZ=25000   # system frequency (applied if low LOW_POWER is set to true)
    PAYLOAD_SIZE=5               # header + value = 1 + sizeof(float)
    PIPE_READ=1                  # Reading pipe
    ADDRESS_READ="CCCC2"
    WRITING_ADDRESS="CCCC1"
)
target_sources(dragon PRIVATE
    main.cpp
)
target_include_directories(dragon PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(dragon)
# Enable usb output, disable uart output
pico_enable_stdio_usb(dragon 1)
pico_enable_stdio_uart(dragon 0)

add_subdirectory(PC)
