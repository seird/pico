#ifndef __ECHO_H__
#define __ECHO_H__


#include <pico/stdlib.h>

#include "utils/pin.h"


class Echo
{
public:
    Echo(uint pin_trigger, uint pin_echo);

    float measure();

private:
    Pin pin_trigger;
    Pin pin_echo;

    const float sound_speed_us = 343 * 1e-6;
};



#endif // __ECHO_H__
