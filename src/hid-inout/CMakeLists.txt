# -----------------------------------------------
# ---------------- hid-inout ----------------
# -----------------------------------------------
add_executable(hid-inout)


target_link_libraries(hid-inout 
    pico_stdlib tinyusb_device tinyusb_board
)
target_compile_definitions(hid-inout PRIVATE
)
target_sources(hid-inout PRIVATE
    main.c
    usb_descriptors.c
)
target_include_directories(hid-inout PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(hid-inout)
# Enable usb output, disable uart output
pico_enable_stdio_usb(hid-inout 1)
pico_enable_stdio_uart(hid-inout 0)
