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
        /**
         * @brief 
         * 
         * @param high 
         * @param duration pulse duration in microseconds
         * @param pre !high level duration before the pulse in microseconds
         * @param post !high level duration after the pulse in microseconds
         */
        void pulse(bool high, uint64_t duration, uint64_t pre = 0, uint64_t post = 0);
        void pull_down();
        void pull_up();
        bool get();
        void blink(int times, uint32_t interval_ms, bool inverted=false);
};


#endif // __PIN_H__
