#include "../../modules/pico-onewire/api/one_wire.h"
#include "temperature.h"


One_wire * one_wire = NULL; // https://github.com/adamboardman/pico-onewire


TemperatureSensor::TemperatureSensor(uint data_pin) : One_wire(data_pin)
{

}


float
TemperatureSensor::measure()
{
    rom_address_t address{};
    single_device_read_rom(address);
    convert_temperature(address, true, false);
    float T = temperature(address);
    if (T >= 100.0f) {
        T = 100.0f;
    } else if (T < 0) {
        T = 0;
    }
    return T;
}
