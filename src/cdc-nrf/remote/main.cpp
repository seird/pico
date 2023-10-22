#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>

#include "utils/pin.h"
#include "utils/radio.h"
#include "echo.h"


enum Header {
    Distance = 0,
    Light    = 1,
};


Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);
Echo echo(PIN_TRIGGER, PIN_ECHO);
Radio radio;
queue_t queue;


bool radio_init();
void loop_function(void * arg);
void core1(void);


bool
radio_init()
{
    if (!radio.setup(PAYLOAD_SIZE))
        return false;

    uint8_t writing_address[] = WRITING_ADDRESS;
    radio.openWritingPipe(writing_address);
    radio.startListening();
    radio.stopListening();
    radio.powerDown();

    return true;
}


void
core1(void)
{
    uint8_t data[PAYLOAD_SIZE];

    while (true) {
        // Distance
        data[0] = Header::Distance;
        *(float *)(data + 1) = echo.measure();
        queue_try_add(&queue, data);

        sleep_ms(100);

        // pin_LED.blink(1, 20);
    }
}


void
loop_function(void * arg)
{
    (void) arg;
    
    uint8_t data[PAYLOAD_SIZE];
    if (queue_try_remove(&queue, data)) {
        radio.powerUp(); // up to 5ms
        radio.send_packet(data);
        radio.powerDown();
    }
}


int
main()
{
    pin_LED.off();
    if (!radio_init())
        pin_LED.on();
    pin_LED.off();

#if LOW_POWER 
// Could do everything on the main core for more efficiency
    if (!set_sys_clock_khz(SYSTEM_FREQUENCY_KHZ, true))
        while (true)
            pin_LED.blink(1, 1000);
#endif

    pin_LED.blink(5, 200); // without a delay here, the program doesn't run after powering a 2nd time.. ??

    queue_init(&queue, PAYLOAD_SIZE, 100);

    multicore_reset_core1();
    multicore_launch_core1(&core1);
    
    while (true)
        loop_function(NULL);

    return 0;
}
