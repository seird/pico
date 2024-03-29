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
        void pack(uint8_t * packet, uint8_t header, uint8_t * data, uint8_t size_data);
        void unpack(uint8_t * header, uint8_t * data, uint8_t size_data, uint8_t * packet);
        bool send_packet(uint8_t * packet);
        bool send_packet_burst(uint8_t * packet, size_t n, uint32_t interval_ms);
        bool send_packets(uint8_t * packets, uint8_t npackets);
        bool send_packets_burst(uint8_t * packets, uint8_t npackets, size_t n, uint32_t interval);
        bool receive_packet(uint8_t * packet, uint8_t * pipe);
        bool receive_packet_timeout(uint8_t * packet, uint8_t * pipe, uint32_t timeout_us);
        bool receive_packets_timeout(uint8_t * packets, uint8_t npackets, uint8_t * pipe, uint32_t timeout_us);
};


#endif // __RADIO_H__
