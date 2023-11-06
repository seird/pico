# -----------------------------------------------
# ---------------- relay-transmitter ----------------
# -----------------------------------------------
add_executable(relay_transmitter)


target_link_libraries(relay_transmitter relay-switch-common sleep radio pin
    hardware_adc
)
target_compile_definitions(relay_transmitter PRIVATE
    BURST=true
    BLINK_LED=true
    PAYLOAD_SIZE=1
    LOW_POWER=true
    PIN_BTN=17                      # Button pin
    SYSTEM_FREQUENCY_KHZ=25000      # system frequency (applied if low LOW_POWER is set to true)
    WRITING_ADDRESS=ADRESS_TR_TO_RX # send the measurement to this address
)
target_sources(relay_transmitter PRIVATE
    main.cpp
)
pico_add_extra_outputs(relay_transmitter)
# Enable usb output, disable uart output
pico_enable_stdio_usb(relay_transmitter 1)
pico_enable_stdio_uart(relay_transmitter 0)
