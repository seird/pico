# -----------------------------------------------
# ---------------- cdc-inout ----------------
# -----------------------------------------------
add_executable(cdc-inout)


target_link_libraries(cdc-inout 
    pico_stdlib tinyusb_device tinyusb_board
)
target_compile_definitions(cdc-inout PRIVATE
)
target_sources(cdc-inout PRIVATE
    main.cpp
    usb_descriptors.cpp
)
target_include_directories(cdc-inout PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(cdc-inout)
# Enable usb output, disable uart output
pico_enable_stdio_usb(cdc-inout 1)
pico_enable_stdio_uart(cdc-inout 0)
