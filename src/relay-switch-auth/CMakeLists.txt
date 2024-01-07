add_library(relay-switch-auth-common INTERFACE)
target_compile_definitions(relay-switch-auth-common INTERFACE
    PAYLOADSIZE=17 # Header (1) + Data (16)
    ADRESS_R_TO_S="4Node"
    ADRESS_S_TO_R="5Node"
)

add_subdirectory(relay-receiver)
add_subdirectory(relay-transmitter)
