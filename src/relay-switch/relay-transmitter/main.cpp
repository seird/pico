#include <stdio.h>
#include <pico/stdlib.h>

#include "utils/pin.hpp"
#include "utils/radio.hpp"
#include "utils/sleep.h"
#include "commands.h"


Radio radio;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);


static bool
setup()
{
    stdio_init_all();

    /* pins */
    pin_LED.off();

    /* Radio */
    if (!radio.setup(sizeof(Command))) {
        return false;
    }
    uint8_t writing_address[] = WRITING_ADDRESS;
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


static void
transmit_command(Command cmd)
{
    radio.powerUp();

#if BURST
    radio.send_packet_burst((uint8_t *)&cmd, BURST_N, BURST_INTERVAL_MS);
#else
    radio.send_packet((uint8_t *)&cmd);
#endif

    radio.powerDown();
}


static void
loop_function(void * arg)
{
    (void) arg;

#if BLINK_LED
    pin_LED.blink(1, 20);
#endif
    transmit_command(CMD_TOGGLE);
#if BLINK_LED
    pin_LED.blink(1, 200);
#endif
}


int
main()
{
    if (!setup()) {
        /* setup failed, blink the pico LED */
        while (true) {
            pin_LED.blink(1, 100);
        }
    }

    sleepconfig_t sleepconfig = {
        .mode = SM_DORMANT,
        .loop_function = &loop_function,
        .pin_wakeup = PIN_BTN,
        .edge = true,
        .active = true,
    };

    sleep_run(&sleepconfig);

    return 0;
}
