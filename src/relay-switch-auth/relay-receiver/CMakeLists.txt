# -----------------------------------------------
# ---------------- relay-receiver-auth ----------------
# -----------------------------------------------
add_executable(relay_receiver_auth)


target_link_libraries(relay_receiver_auth relay-switch-auth-common sleep radio pin crypto
    hardware_adc
)
target_compile_definitions(relay_receiver_auth PRIVATE
    BLINK_LED=false
    PAYLOAD_SIZE=1
    PIN_RELAY=0
    LOW_POWER=false
    SYSTEM_FREQUENCY_KHZ=25000   # system frequency (applied if low LOW_POWER is set to true)
    PIPE_READ=1                  # Reading pipe
    ADDRESS_READ=ADRESS_R_TO_S   # Read from the sensor at this address
    ADDRESS_WRITE=ADRESS_S_TO_R
)
target_sources(relay_receiver_auth PRIVATE
    main.cpp
)
pico_add_extra_outputs(relay_receiver_auth)
# Enable usb output, disable uart output
pico_enable_stdio_usb(relay_receiver_auth 1)
pico_enable_stdio_uart(relay_receiver_auth 0)
