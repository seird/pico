#include <stdio.h>
#include <pico/stdlib.h>
#include "tusb.h"

#include "utils/sts.hpp"
#include "utils/utils.h"
#include "utils/pin.hpp"
#include "utils/radio.hpp"
#include "utils/sleep.h"
#include "commands.h"
#include "key.h"


Radio radio;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);
struct STS sts;

const uint8_t address_read_node[6] = ADDRESS_READ;
const uint8_t address_write[] = ADDRESS_WRITE;

static bool
setup()
{
    /* pins */
    pin_LED.off();

    /* Radio */
    if (!radio.setup(PAYLOADSIZE)) {
        return false;
    }
    
    radio.openWritingPipe(address_write);
    radio.openReadingPipe(PIPE_READ, address_read_node);
    radio.startListening();
    radio.stopListening();
    radio.powerDown();

    sts_set_key(&sts, PRESHARED_KEY);
    sts_init(&sts); // (pub, prv) transmitter can be safely re-used (?)

    /* Set system clock */
#if LOW_POWER
    if (!set_sys_clock_khz(SYSTEM_FREQUENCY_KHZ, true)) {
        return false;
    }
#endif

    return true;
}


static void
transmit_command(Command command)
{
    printf("%s\n", __func__);

    uint32_t t = time_us_32();

    radio.powerUp();

    radio.flush_rx();
    radio.flush_tx();

    if (!sts1_tx(&radio, &sts)) {
        printf("sts1_tx failed\n");
        goto finish;
    }

    if (!sts2_rx(&radio, &sts)) {
        printf("sts2_rx failed\n");
        goto finish;
    }

    printf("sts->s: "); print_array(sts.s, sizeof(sts.s));
    printf("sts->iv: "); print_array(sts.iv, sizeof(sts.iv));

    if (!sts3_tx(&radio, &sts)) {
        printf("sts3_tx failed\n");
        goto finish;
    }

    // Send encrypted Command
    if (!cmd_tx(&radio, &sts, command)) {
        printf("cmd_tx failed\n");
        goto finish;
    }

    // Wait for ACK
    if (!ack_rx(&radio, &sts)) {
        printf("ack_rx failed\n");
        goto finish;
    }

#if BLINK_LED
    pin_LED.blink(1, 1000); // Success
#endif

finish:
    printf("Took %f seconds\n", (time_us_32()-t)/1e6);

    radio.flush_rx();
    radio.flush_tx();
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
}


int
main()
{
    stdio_init_all();

    if (!setup()) {
        /* setup failed, blink the pico LED */
        while (true) {
            pin_LED.blink(1, 100);
        }
    }

#if defined(LOW_POWER) && LOW_POWER == 1
    sleepconfig_t sleepconfig = {
        .mode = SM_DORMANT,
        .loop_function = &loop_function,
        .pin_wakeup = PIN_BTN,
        .edge = true,
        .active = true,
    };
    sleep_run(&sleepconfig);
#else
    while (!tud_cdc_connected()) {
        sleep_ms(10);
    }

    printf("Hello world! Press 's' to send a command.\n");

    while (true) {
        int c = getchar();
        switch (c) {
            case 's':
                transmit_command(CMD_TOGGLE);
                break;
            case 'd':
                transmit_command(CMD_FORCE);
                break;
            default:
                break;
        }
        sleep_ms(100);
    }
#endif


    return 0;
}
