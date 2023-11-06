# -----------------------------------------------
# ---------------- cdc-nrf-remote ----------------
# -----------------------------------------------
add_executable(cdc-nrf-remote)


target_link_libraries(cdc-nrf-remote cdc-nrf-common pin radio
    pico_stdlib pico_multicore tinyusb_device tinyusb_board
)
target_compile_definitions(cdc-nrf-remote PRIVATE
    PIN_TRIGGER=0
    PIN_ECHO=1
    PAYLOAD_SIZE=5                       # header + value = 1 + sizeof(float)
    LOW_POWER=true
    SYSTEM_FREQUENCY_KHZ=25000           # system frequency (applied if low LOW_POWER is set to true)
    WRITING_ADDRESS=ADDRESS_REMOTE_TO_PC # Transmit to this address
)
target_sources(cdc-nrf-remote PRIVATE
    main.cpp
    echo.cpp
)
target_include_directories(cdc-nrf-remote PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(cdc-nrf-remote)
# Enable usb output, disable uart output
pico_enable_stdio_usb(cdc-nrf-remote 1)
pico_enable_stdio_uart(cdc-nrf-remote 0)