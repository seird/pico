add_library(relay-switch-common INTERFACE)
target_compile_definitions(relay-switch-common INTERFACE
    BURST=true
    BURST_N=25
    BURST_INTERVAL_MS=5
    ADRESS_TR_TO_RX="2Node"
)

add_subdirectory(relay-receiver)
add_subdirectory(relay-transmitter)
