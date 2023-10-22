# -----------------------------------------------
# ---------------- cdc-temperature ----------------
# -----------------------------------------------
add_executable(cdc-temperature)

target_link_libraries(cdc-temperature common 
    pico_stdlib tinyusb_device tinyusb_board pico_one_wire hardware_irq hardware_i2c
)
target_compile_definitions(cdc-temperature PRIVATE
    PIN_TEMPERATURE=17          # Data pin of the DS18B20 sensor
)
target_sources(cdc-temperature PRIVATE
    main.cpp
    usb_descriptors.cpp
    temperature.cpp
)
target_include_directories(cdc-temperature PUBLIC ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(cdc-temperature)
# Enable usb output, disable uart output
pico_enable_stdio_usb(cdc-temperature 1)
pico_enable_stdio_uart(cdc-temperature 0)
