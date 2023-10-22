# -----------------------------------------------
# ---------------- cdc-nrf-pc ----------------
# -----------------------------------------------
add_executable(cdc-nrf-pc)


target_link_libraries(cdc-nrf-pc common pin radio
    pico_stdlib pico_multicore tinyusb_device tinyusb_board
)
target_compile_definitions(cdc-nrf-pc PRIVATE
    PAYLOAD_SIZE=5              # header + value = 1 + sizeof(float)
    LOW_POWER=false
    SYSTEM_FREQUENCY_KHZ=25000  # system frequency (applied if low LOW_POWER is set to true)
    PIPE_READ=1                 # Reading pipe
    ADDRESS_READ="AAAA1" 
)
target_sources(cdc-nrf-pc PRIVATE
    main.cpp
    usb_descriptors.cpp
)
target_include_directories(cdc-nrf-pc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(cdc-nrf-pc)
# Enable usb output, disable uart output
pico_enable_stdio_usb(cdc-nrf-pc 1)
pico_enable_stdio_uart(cdc-nrf-pc 0)