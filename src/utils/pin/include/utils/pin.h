#ifndef __PIN_H__
#define __PIN_H__


#include <pico/stdlib.h>


class Pin {
    private:
        uint pin;
        bool direction;
    public:
        Pin(uint n, bool direction);
        void on();
        void off();
        void set(bool state);

        /**
         * @brief Toggle a pin. Returns the new state of the pin.
         */
        bool toggle();
        void pull_down();
        void pull_up();
        bool get();
        void blink(int times, uint32_t interval_ms, bool inverted=false);
};


#endif // __PIN_H__
