#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__


#include <pico/stdlib.h>
#include "pico-onewire/api/one_wire.h"


class TemperatureSensor : public One_wire {
    public:
        TemperatureSensor(uint data_pin);
        float measure();
};


#endif // __TEMPERATURE_H__
