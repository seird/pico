add_library(cdc-nrf-common INTERFACE)
target_compile_definitions(cdc-nrf-common INTERFACE
    ADDRESS_REMOTE_TO_PC="AAAA1"
)

add_subdirectory(PC)
add_subdirectory(remote)
