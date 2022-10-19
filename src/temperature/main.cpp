#include <stdio.h>
#include <pico/stdlib.h>

#include "../pin/pin.h"
#include "../radio/radio.h"
#include "../sleep/sleep.h"
#include "temperature.h"


/* ----------------------------------------- Parameters ----------------------------------------- */
#define PIN_TEMPERATURE 0            // Data pin of the DS18B20 sensor
#define MEASURE_EVERY_S 60           // perform a measurement every X seconds
#define TEMPERATURE_SMOOTHING 1      // average the last X temperature measurements
uint8_t writing_address[] = "2Node"; // send the measurement to this address
#define SYSTEM_FREQUENCY_KHZ 13000   // system frequency (applied if low LOW_POWER is set to true)
/* ---------------------------------------------------------------------------------------------- */


float measurements[TEMPERATURE_SMOOTHING];

Radio radio;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);
TemperatureSensor temperature_sensor(PIN_TEMPERATURE);


static bool
setup()
{
    stdio_init_all();

    /* LED */
    pin_LED.off();

    /* Temperature sensor */
    temperature_sensor.init();

    /* Radio */
    if (!radio.setup(sizeof(float))) {
        return false;
    }
    radio.openWritingPipe(writing_address);
    radio.startListening();
    radio.stopListening();
    radio.powerDown();

    /* Set system clock */
#if LOW_POWER
    if (!set_sys_clock_khz(SYSTEM_FREQUENCY_KHZ, true)) {
        return false;
    }
#endif

    return true;
}


static float
average(float * array, size_t size)
{
    float sum = 0.0f;
    for (int i=0; i<size; ++i) {
        sum += array[i];
    }
    return sum / size;
}


static void
loop_function(void * arg)
{
    (void) arg;

    size_t * m = (size_t *) arg;

    /* measure temperature */
#if LOW_POWER
    // The temperature sensor has to be reinitialized every time the pico wakes from sleep
    temperature_sensor.init();
#endif
    measurements[(*m)++ % TEMPERATURE_SMOOTHING] = temperature_sensor.measure();

    /* update the smoothed temperature */
    float temperature = average(measurements, TEMPERATURE_SMOOTHING);
    // printf("[m = %3d] %f\n", *m, temperature);

    /* transmit the temperature */
    radio.transmit_temperature(temperature);

#if BLINK_LED
    pin_LED.blink(1, 100);
#endif
}


int
main()
{
    if (!setup()) {
        /* setup failed, blink the pico LED */
        while (true) {
            pin_LED.blink(1, 500);
        }
    }

    /* fill the measurements array with initial values */
    temperature_sensor.measure();
    float value = temperature_sensor.measure();
    size_t m = 0;
    for (m=0; m<TEMPERATURE_SMOOTHING; ++m) {
        measurements[m] = value;
    }

    sleepconfig_t sleepconfig = {
        .mode = LOW_POWER ? SM_SLEEP : SM_DEFAULT,
        .hours = 0,
        .minutes = 0,
        .seconds = MEASURE_EVERY_S,
        .loop_function = loop_function,
        .arg = &m, // could use a global variable as well, however `static size_t m = 0;` inside loop_function is always 0??
    };

    sleep_run(&sleepconfig);

    return 0;
}
