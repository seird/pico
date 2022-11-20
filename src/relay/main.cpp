#include <pico/stdlib.h>
#include <pico/multicore.h>

#include "../pin/pin.h"
#include "../radio/radio.h"


// Node A = temperature sensor
// Node B = server

const uint8_t address_read_node_A[6] = ADDRESS_READ_A;
const uint8_t address_write_node_B[6] = ADDRESS_WRITE_B;
const uint8_t address_write_node_A[6] = "XXXXA"; // We're only passing packets from A to B
const uint8_t address_read_node_B[6] = "XXXXB"; // We're only passing packets from A to B

Radio radio;
Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);

bool blink_error = false;
bool blink_radio_failure = false;
bool blink_traffic = false;


static bool
radio_init()
{
    if (!radio.setup(sizeof(float))) {
        return false;
    }

    radio.openReadingPipe(PIPE_A, address_read_node_A);
    radio.openReadingPipe(PIPE_B, address_read_node_B);

    radio.startListening(); // set the radio in RX mode
    
    return true;
}


static bool
setup()
{
    stdio_init_all();

    sleep_ms(100);

    /* LED */
    pin_LED.off();

    /* Set system clock */
#if LOW_POWER // TODO: multi-core ?
    if (!set_sys_clock_khz(SYSTEM_FREQUENCY_KHZ, true)) {
        return false;
    }
#endif

    /* set up the radio */
    return radio_init();
}


void
core1(void)
{
    while (true) {
        if (blink_traffic) {
            pin_LED.blink(1, 50);
            blink_traffic = false;
        }

        if (blink_error) {
            pin_LED.blink(2, 200);
            blink_error = false;
        }

        if (blink_radio_failure) { // blink fast for 10 seconds
            pin_LED.blink(100, 100);
            blink_radio_failure = false;
        }

        sleep_ms(10);
    }

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

    // We are only expecting packets on PIPE_A or PIPE_B (relay between 2 nodes)
    if (pipe != PIPE_A && pipe != PIPE_B) {
        blink_error = true;
        return;
    }

    // Pass the packet on to the next node
    // Pipe 1 receives from A --> write to B
    // Pipe 2 receives from B --> write to A
    switch (pipe) {
        case PIPE_A:
            radio.openWritingPipe(address_write_node_B);
            break;
        case PIPE_B:
            radio.openWritingPipe(address_write_node_A);
            break;
        default:
            blink_error = true;
            return;
    }

    // printf("Received temperature %f on pipe %d\n", *(float *)buffer, pipe);

    radio.send_packets(buffer, 1);
    blink_traffic = true;
    radio.startListening();
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

#if BLINK_CORE
    multicore_launch_core1(&core1);
#endif

    while (true) {
    #if HANDLE_RADIO_FAILURES
        if (radio.failureDetected) {
            blink_radio_failure = true;
            radio_init();
        }
    #endif

        loop_function(NULL);

        sleep_ms(INTERVAL_MS);
    }
}
