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

#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "utils/radio.h"
#include "utils/pin.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

enum Command {
    ResetCore1 = 0x1,
    BlinkTraffic = 0x2,
};

void cdc_task(void);
void core1(void);
bool radio_init();

Pin pin_LED(PICO_DEFAULT_LED_PIN, GPIO_OUT);
Radio radio;
queue_t queue;
static bool flashLED = false;

const uint8_t address_read_node[6] = ADDRESS_READ;


/*------------- MAIN -------------*/
int main(void)
{
    board_init();
    queue_init(&queue, PAYLOAD_SIZE, 100);

    pin_LED.off();
    while (!radio_init())
        pin_LED.on();
    pin_LED.off();

    multicore_launch_core1(&core1);
    
    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    while (1) {
        tud_task(); // tinyusb device task

        cdc_task();
    }
}

bool
radio_init() {
    if (!radio.setup(PAYLOAD_SIZE))
        return false;
    
    radio.openReadingPipe(PIPE_READ, address_read_node);
    radio.startListening(); // set the radio in RX mode
    
    return true;
}


// Radio core
void core1(void)
{
    uint8_t buffer[PAYLOAD_SIZE];
    uint8_t pipe;

    while (true) {
        // Check for a new packet
        if (radio.receive_packet(buffer, &pipe)) {
            queue_try_add(&queue, buffer);
            if (flashLED)
                pin_LED.blink(1, 5);
        }

        sleep_ms(10);
    }
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
    // Process received data from USB host
    if (tud_cdc_available()) {
        // read data
        char buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));

        if (count == 1) {
            switch (Command(*buf))
            {
            case ResetCore1:
                multicore_reset_core1();
                radio_init();
                multicore_launch_core1(&core1);
                break;
            case BlinkTraffic:
                flashLED = !flashLED;
                break;
            default:
                break;
            }
        }
    }

    // Send data to USB host
    uint8_t data[PAYLOAD_SIZE];
    if (queue_try_remove(&queue, data)) { // Always remove the data from the queue, even if there is no usb host connected
        if (tud_cdc_connected()) {
            tud_cdc_write(data, PAYLOAD_SIZE);
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
