# -----------------------------------------------
# ------------- measure-temperature -------------
# -----------------------------------------------
add_executable(measure_temperature)

add_subdirectory(pico-onewire)

target_link_libraries(measure_temperature common sleep radio pin
    hardware_irq
    hardware_i2c
    pico_one_wire
)
target_compile_definitions(measure_temperature PRIVATE
    BLINK_LED=true
    LOW_POWER=true
    PAYLOAD_SIZE=5 # Header (1) + Data (4)
    PIN_TEMPERATURE=17          # Data pin of the DS18B20 sensor
    MEASURE_EVERY_S=0           # perform a measurement every X seconds [0-59]
    MEASURE_EVERY_M=1           # perform a measurement every X minutes [0-59]
    MEASURE_EVERY_H=0           # perform a measurement every X hours [0-23]
    TEMPERATURE_SMOOTHING=1     # average the last X temperature measurements
    SYSTEM_FREQUENCY_KHZ=13000  # system frequency (applied if low LOW_POWER is set to true)
    WRITING_ADDRESS="2Node"     # send the measurement to this address
    PACKET_BURST=10             # send bursts of X packets
    PACKET_BURST_INTERVAL_MS=20 # time in ms between packets in a single burst
    HEADER=0                    # header to add to the radio packet
)
target_sources(measure_temperature PRIVATE
    main.cpp
    temperature.cpp
)
pico_add_extra_outputs(measure_temperature)
# Enable usb output, disable uart output
pico_enable_stdio_usb(measure_temperature 1)
pico_enable_stdio_uart(measure_temperature 0)
