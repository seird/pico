# -----------------------------------------------
# ---------------- dragon-pc ----------------
# -----------------------------------------------
add_executable(dragon-pc)


target_link_libraries(dragon-pc pin radio
    pico_stdlib pico_multicore tinyusb_device tinyusb_board
)
target_compile_definitions(dragon-pc PRIVATE
    PAYLOAD_SIZE=5                      # header + value = 1 + sizeof(float)
    LOW_POWER=false
    SYSTEM_FREQUENCY_KHZ=25000          # system frequency (applied if low LOW_POWER is set to true)
    PIPE_READ=1                         # Reading pipe
    ADDRESS_READ="CCCC1"
    WRITING_ADDRESS="CCCC2"
)
target_sources(dragon-pc PRIVATE
    main.cpp
    usb_descriptors.cpp
)
target_include_directories(dragon-pc PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(dragon-pc)
# Enable usb output, disable uart output
pico_enable_stdio_usb(dragon-pc 1)
pico_enable_stdio_uart(dragon-pc 0)