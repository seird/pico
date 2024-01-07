# -----------------------------------------------
# ---------------- relay-transmitter-auth ----------------
# -----------------------------------------------
add_executable(relay_transmitter_auth)


target_link_libraries(relay_transmitter_auth relay-switch-auth-common sleep radio pin crypto
    hardware_adc
)
target_compile_definitions(relay_transmitter_auth PRIVATE
    BLINK_LED=true
    PAYLOAD_SIZE=1
    LOW_POWER=true
    PIN_BTN=17                      # Button pin
    SYSTEM_FREQUENCY_KHZ=25000      # system frequency (applied if low LOW_POWER is set to true)
    PIPE_READ=1
    ADDRESS_WRITE=ADRESS_R_TO_S   # send the measurement to this address
    ADDRESS_READ=ADRESS_S_TO_R   # send the measurement to this address
)
target_sources(relay_transmitter_auth PRIVATE
    main.cpp
)
pico_add_extra_outputs(relay_transmitter_auth)
# Enable usb output, disable uart output
pico_enable_stdio_usb(relay_transmitter_auth 1)
pico_enable_stdio_uart(relay_transmitter_auth 0)
