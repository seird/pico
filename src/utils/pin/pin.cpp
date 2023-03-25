#include "utils/pin.h"


Pin::Pin(uint n, bool direction)
{
    pin = n;
    direction = direction;

    gpio_init(n);
    gpio_set_dir(n, direction);
}


void
Pin::on()
{
    gpio_put(pin, true);
}


void
Pin::off()
{
    gpio_put(pin, false);
}


void
Pin::pull_down()
{
    gpio_pull_down(pin);
}


void
Pin::pull_up()
{
    gpio_pull_up(pin);
}


bool
Pin::get()
{
    return gpio_get(pin);
}


void
Pin::blink(int times, uint32_t interval_ms, bool inverted)
{
    for (int i=0; i<times; ++i) {
        if (inverted) {
            off();
        } else {
            on();
        }

        sleep_ms(interval_ms);

        if (inverted) {
            on();
        } else {
            off();
        }

        if (i < (times-1)) {
            sleep_ms(interval_ms);
        }
    }
}
