# -----------------------------------------------
# ---------------- relay-receiver ----------------
# -----------------------------------------------
add_executable(relay_receiver)


target_link_libraries(relay_receiver common sleep radio pin
    hardware_adc
)
target_compile_definitions(relay_receiver PRIVATE
    BLINK_LED=false
    PAYLOAD_SIZE=1
    PIN_RELAY=0
    LOW_POWER=false
    SYSTEM_FREQUENCY_KHZ=25000  # system frequency (applied if low LOW_POWER is set to true)
    PIPE_READ=1                    # Reading pipe
    ADDRESS_READ="2Node"        # Read from the sensor at this address
)
target_sources(relay_receiver PRIVATE
    main.cpp
)
pico_add_extra_outputs(relay_receiver)
# Enable usb output, disable uart output
pico_enable_stdio_usb(relay_receiver 1)
pico_enable_stdio_uart(relay_receiver 0)
