#ifndef __RADIO_H__
#define __RADIO_H__

/*
    https://nrf24.github.io/RF24/
    https://nrf24.github.io/RF24/md_docs_pico_sdk.html
*/

#include <pico/stdlib.h>
#include <RF24.h>         // RF24 radio object


class Radio : public RF24 {
    public:
        // Radio() : RF24(CE_PIN, CSN_PIN) {};
        Radio();
        bool setup(uint8_t payload_size);
        void send_packets(uint8_t * packets, uint8_t npackets);
        bool receive_packet(uint8_t * packet, uint8_t * pipe);
        bool receive_packet_timeout(uint8_t * packet, uint8_t * pipe, uint32_t timeout_us);
        bool receive_packets_timeout(uint8_t * packets, uint8_t npackets, uint8_t * pipe, uint32_t timeout_us);
        void transmit_temperature(float temperature);
};


#endif // __RADIO_H__
