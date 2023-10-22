#include "echo.h"


Echo::Echo(uint pin_trigger, uint pin_echo) :
    pin_trigger(pin_trigger, GPIO_OUT),
    pin_echo(pin_echo, GPIO_IN)
{
    this->pin_echo.pull_down();
}


float Echo::measure()
{
    // Send a high pulse to the trigger pin
    pin_trigger.pulse(true, 10, 10, 0);
    
    // Read the signal from the echo pin:
    // - The signal is a HIGH pulse
    // - The duration that the pulse is high is the time between the trigger being sent and the echo being received
    uint32_t t_start;
    do {
        t_start = time_us_32();
    } while (pin_echo.get() == false);

    uint32_t t_end;
    do {
        t_end = time_us_32();
    } while (pin_echo.get() == true);

    uint32_t duration_us = t_end - t_start; // microseconds
    float distance = 0.5f * sound_speed_us * duration_us; // meters

    return distance;
}
