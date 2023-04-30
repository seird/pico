# -----------------------------------------------
# -------------------- relay --------------------
# -----------------------------------------------
add_executable(relay)
target_link_libraries(relay common radio pin
    pico_multicore
)
target_compile_definitions(relay PRIVATE
    BLINK_CORE=true
    HANDLE_RADIO_FAILURES=true
    LOW_POWER=false
    SYSTEM_FREQUENCY_KHZ=13000  # system frequency (applied if low LOW_POWER is set to true)
    ADDRESS_READ_A="2Node"      # Read from the sensor at this address
    PIPE_A=1                    # Reading pipe
    ADDRESS_WRITE_B="1Node"     # Write to the base station at this address
    PIPE_B=2                    # Writing pipe
    INTERVAL_MS=50              # Execute the relay main loop every X ms
    PACKET_BURST=10             # send bursts of X packets
    PACKET_BURST_INTERVAL_MS=20 # time in ms between packets in a single burst
)
target_sources(relay PRIVATE
    main.cpp
)
pico_add_extra_outputs(relay)
# Enable usb output, disable uart output
pico_enable_stdio_usb(relay 1)
pico_enable_stdio_uart(relay 0)
