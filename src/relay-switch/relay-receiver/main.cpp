#include <stdio.h>
#include <pico/stdlib.h>

#include "utils/pin.h"
#include "utils/radio.h"
#include "utils/sleep.h"
#include "commands.h"


Radio radio;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);
Pin pin_relay(PIN_RELAY, GPIO_OUT);


const uint8_t address_read_node[6] = ADDRESS_READ;


static bool
radio_init()
{
    if (!radio.setup(sizeof(Command))) {
        return false;
    }

    radio.openReadingPipe(PIPE_READ, address_read_node);
    radio.startListening(); // set the radio in RX mode
    
    return true;
}


static bool
setup()
{
    stdio_init_all();

    /* pins */
    pin_LED.off();
    pin_relay.off();

    /* set up the radio */
    if (!radio_init()) {
        return false;
    }

    /* Set system clock */
#if LOW_POWER
    if (!set_sys_clock_khz(SYSTEM_FREQUENCY_KHZ, true)) {
        return false;
    }
#endif

    return true;
}


static void
loop_function(void * arg)
{
    (void) arg;

    uint8_t buffer[radio.getPayloadSize()];
    uint8_t pipe;

    // Check for a new packet
    if (!radio.receive_packet(buffer, &pipe)) {
        return;
    }

    Command cmd = *(Command *)buffer;

    printf("Received bytes: %x  [%d]\n", *buffer, cmd);
 
    switch(cmd) {
        case CMD_TOGGLE: {
            bool state = pin_relay.toggle();
            pin_LED.set(state);
            break;
        }
        default:
            printf("Unknown command %d\n", cmd);
            break;
    }
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

    while(true) {
        loop_function(NULL);
        sleep_ms(10);
    }

    return 0;
}
