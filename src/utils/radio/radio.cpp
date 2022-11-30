#include "utils/radio.h"

#define CE_PIN 14
#define CSN_PIN 15
#define IRQ_PIN 6


Radio::Radio() : RF24(CE_PIN, CSN_PIN) {
}


bool
Radio::setup(uint8_t payload_size)
{
    // initialize the transceiver on the SPI bus
    if (!begin()) {
        return false;
    }

    setPayloadSize(payload_size);

    if (isPVariant()){
        setDataRate(RF24_250KBPS); // RF24_1MBPS default
    }
    setCRCLength(RF24_CRC_16); // RF24_CRC_16 default

    failureDetected = 0; 

    return true;
}


void
Radio::pack(uint8_t * packet, uint8_t header, uint8_t * data, uint8_t size_data)
{
    *(packet) = header;
    memcpy(packet + 1, data, size_data);
}


void
Radio::unpack(uint8_t * header, uint8_t * data, uint8_t size_data, uint8_t * packet)
{
    *header = *packet;
    memcpy(data, packet + 1, size_data);
}


void
Radio::send_packets(uint8_t * packets, uint8_t npackets)
{
    stopListening();
    uint8_t packet_size = getPayloadSize();
    for (uint8_t i=0; i<npackets; ++i) {
        write(packets + i*packet_size, packet_size);
    }
}


bool
Radio::receive_packet(uint8_t * packet, uint8_t * pipe)
{
    if (available(pipe)) {                // is there a payload? get the pipe number that recieved it
        uint8_t bytes = getPayloadSize(); // get the size of the payload
        read(packet, bytes);              // fetch payload from FIFO
        return true;
    }
    return false;
}


bool
Radio::receive_packet_timeout(uint8_t * packet, uint8_t * pipe, uint32_t timeout_us)
{
    uint32_t t_start = time_us_32();
    while (true) {
        if (receive_packet(packet, pipe)) {
            return true;
        }

        if ((time_us_32() - t_start) > timeout_us) {
            return false;
        }
    }
}


bool
Radio::receive_packets_timeout(uint8_t * packets, uint8_t npackets, uint8_t * pipe, uint32_t timeout_us)
{
    uint32_t t_start = time_us_32();
    uint32_t timeout = timeout_us;
    for (uint8_t i=0; i<npackets; ++i) {
        uint32_t t_elapsed = time_us_32() - t_start;
        timeout -= t_elapsed;
        timeout = MAX(timeout, 0);
        
        if (!receive_packet_timeout(packets + i*getPayloadSize(), pipe, timeout)) {
            return false;
        }
    }
    return true;
}
