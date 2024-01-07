#include <stdio.h>
#include <pico/stdlib.h>

#include "utils/sts.hpp"
#include "utils/pin.hpp"
#include "utils/radio.hpp"
#include "utils/sleep.h"
#include "utils/utils.h"
#include "commands.h"
#include "key.h"


Radio radio;
struct STS sts;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);
Pin pin_relay(PIN_RELAY, GPIO_OUT);
long last_received = 0;


const uint8_t address_read_node[6] = ADDRESS_READ;
const uint8_t address_write[] = ADDRESS_WRITE;


static bool
radio_init()
{
    if (!radio.setup(PAYLOADSIZE)) {
        return false;
    }

    radio.openWritingPipe(address_write);
    radio.openReadingPipe(PIPE_READ, address_read_node);
    radio.startListening(); // set the radio in RX mode
    
    return true;
}


static bool
setup()
{
    /* pins */
    pin_LED.off();

    /* set up the radio */
    if (!radio_init()) {
        return false;
    }

    sts_set_key(&sts, PRESHARED_KEY);

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

    uint32_t t = time_us_32();

    if (!sts1_rx(&radio, &sts)) {
        // printf("sts1_rx failed\n");
        goto finish;
    }

    printf("pub: "); print_array(sts.pub, sizeof(sts.pub));
    printf("prv: "); print_array(sts.prv, sizeof(sts.prv));
    printf("sts->s: "); print_array(sts.s, sizeof(sts.s));
    
    if (!sts2_tx(&radio, &sts)) {
        printf("sts2_tx failed\n");
        goto finish;
    }

    printf("sts->iv: "); print_array(sts.iv, sizeof(sts.iv));

    if (!sts3_rx(&radio, &sts)) {
        printf("sts3_rx failed\n");
        goto finish;
    }

    // Wait for Command
    uint8_t command;
    if (!cmd_rx(&radio, &sts, &command)) {
        printf("cmd_rx failed\n");
        goto finish;
    }

    // Send ACK
    if (!ack_tx(&radio, &sts)) {
        printf("ack_tx failed\n");
        goto finish;
    }

    printf("Executing command %d\n", command);
    switch (command)
    {
    case CMD_TOGGLE:
        pin_relay.on();
        sleep_ms(1000);
        pin_relay.off();
        break;
    
    default:
        break;
    }
    printf("Done!!!\n");
    
finish:
    radio.flush_rx();
    radio.flush_tx();
}


int
main()
{
    stdio_init_all();

    while (!setup())
        pin_LED.blink(1, 100);

    while (true) {
        loop_function(NULL);
        sleep_ms(10);
    }

    return 0;
}
