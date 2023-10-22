/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>
#include <hardware/adc.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "echo.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

enum Header {
    Distance = 0,
    Light    = 1,
};

void cdc_task(void);
void core1(void);
uint get_adc_input(uint pin);

queue_t distance_queue;
queue_t light_queue;

Echo echo(PIN_TRIGGER, PIN_ECHO);
Pin pin_photoresistor(PIN_PHOTORESISTOR, GPIO_IN);
/*
GND o---------- 10kOhm ----------o---------- PHOTORESISTOR ----------o 3.3V
                                 |
                                 |
                                 |
                                 o
                         PIN_PHOTORESISTOR
*/


/*------------- MAIN -------------*/
int main(void)
{
    board_init();

    queue_init(&distance_queue, sizeof(float), 1);
    queue_init(&light_queue, sizeof(float), 1);
    
    adc_init();
    adc_gpio_init(PIN_PHOTORESISTOR);
    adc_select_input(get_adc_input(PIN_PHOTORESISTOR));

    multicore_launch_core1(&core1);
    
    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    while (1) {
        tud_task(); // tinyusb device task

        cdc_task();
    }
}


void core1(void)
{
    while (true) {
        // Distance
        float distance = echo.measure();
        queue_try_add(&distance_queue, &distance);

        // Photoresistor
        /**
         * https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#rpipe94a81e06ca005c7f718
         * 
         * - RP2040: 12 bit ADC
         * - adc_read's output is returned as uint16_t
         */
        float light = 100 * (float)adc_read() / 4095; // 2**12 - 1 = 4095

        queue_try_add(&light_queue, &light);

        sleep_ms(100);
    }
}


uint
get_adc_input(uint pin)
{
    // https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#rpipd757de037c951f31658f

    uint input;

    switch (pin) {
        case 26: input = 0; break;
        case 27: input = 1; break;
        case 28: input = 2; break;
        case 29: input = 3; break;
        default: input = 0;
    }

    return input;
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}


//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
    // connected and there are data available
    // Process received data
    if (tud_cdc_available()) {
        // read data
        char buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));

        if (count == 1 && buf[0] == 0x01) {
            // Reset core
            multicore_reset_core1();
            multicore_launch_core1(&core1);
        }
    }

    if (tud_cdc_connected()) {
        float distance;
        if (queue_try_remove(&distance_queue, &distance)) {
            char header = Header::Distance;
            tud_cdc_write(&header, sizeof(header));
            tud_cdc_write(&distance, sizeof(distance));
            tud_cdc_write_flush();
        }

        float light;
        if (queue_try_remove(&light_queue, &light)) {
            char header = Header::Light;
            tud_cdc_write(&header, sizeof(header));
            tud_cdc_write(&light, sizeof(light));
            tud_cdc_write_flush();
        }
    }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    board_led_write(dtr);
    if ( dtr ) {
    } else {
    }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
}
