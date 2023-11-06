# -----------------------------------------------
# ---------------- cdc-sensors ----------------
# -----------------------------------------------
add_executable(cdc-sensors)


target_link_libraries(cdc-sensors pin
    pico_stdlib pico_multicore hardware_adc tinyusb_device tinyusb_board
)
target_compile_definitions(cdc-sensors PRIVATE
    PIN_TRIGGER=0
    PIN_ECHO=1
    PIN_PHOTORESISTOR=28
)
target_sources(cdc-sensors PRIVATE
    main.cpp
    usb_descriptors.cpp
    echo.cpp
)
target_include_directories(cdc-sensors PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(cdc-sensors)
# Enable usb output, disable uart output
pico_enable_stdio_usb(cdc-sensors 1)
pico_enable_stdio_uart(cdc-sensors 0)
