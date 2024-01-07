#include "utils/pin.hpp"


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
Pin::set(bool state)
{
    if (state)
        on();
    else
        off();
}


bool
Pin::toggle()
{
    bool state = get(); // current state
    set(!state);
    return !state;
}


void
Pin::pulse(bool high, uint64_t duration, uint64_t pre, uint64_t post)
{
    // -- blink
    //
    // high = true
    //  pre     duration    post
    //        ____________
    //        |          |  
    // _______|          |________
    //
    //
    // high = false
    //
    //  pre     duration    post
    // ________          _________
    //        |          |
    //        |__________|
    //
    //

    set(!high);
    sleep_us(pre);
    set(high);
    sleep_us(duration);
    set(!high);
    sleep_us(post);
    
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

        sleep_ms(interval_ms);
    }
}
