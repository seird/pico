#include <string.h>


#include "utils/sts.hpp"
#include "utils/aes.h"
#include "utils/ecdh.h"
#include "utils/utils.h"
#include "utils/mac.h"


#define HEADER_STS1 0x0
#define HEADER_STS2 0x1
#define HEADER_STS3 0x2
#define HEADER_CMD  0x3
#define HEADER_ACK  0x4

#define BYTE_ACK    0x22

#define STS1_RX_TIMEOUT_MS 5000
#define STS2_RX_TIMEOUT_MS 5000
#define STS3_RX_TIMEOUT_MS 5000
#define CMD_RX_TIMEOUT_MS  5000
#define ACK_RX_TIMEOUT_MS  5000


bool
sts_init(struct STS * sts)
{
    printf("<%s>\n", __func__);

    random_bytes(sts->prv, ECC_PRV_KEY_SIZE);
    return ecdh_generate_keys(sts->pub, sts->prv) == 0;
}


void
sts_set_key(struct STS * sts, const uint8_t * k)
{
    memcpy(sts->k, k, AES_BLOCKLEN);
}


/* ================================================ STS1 ================================================ */

bool
sts1_tx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    uint8_t packet[PAYLOADSIZE];
    radio->pack(packet, HEADER_STS1, sts->pub+0*16, 16);
    radio->send_packet(packet);
    radio->pack(packet, HEADER_STS1, sts->pub+1*16, 16);
    radio->send_packet(packet);
    radio->pack(packet, HEADER_STS1, sts->pub+2*16, 16);
    radio->send_packet(packet);

    radio->startListening();

    return true;
}


bool
sts1_rx(Radio * radio, struct STS * sts)
{
    // printf("<%s>\n", __func__);

    // Receive pubOther & compute the shared secret

    uint8_t packet[PAYLOADSIZE];
    uint8_t pipe;
    uint8_t header;

    uint32_t t = time_us_32();
    int i = 0;
    while (((time_us_32() - t) / 1000) < STS1_RX_TIMEOUT_MS) {
        if (radio->receive_packet(packet, &pipe)) {
            header = *packet;

            if (header != HEADER_STS1) {
                printf("Incorrect header in %s: %d\n", __func__, header);
                return false;
            }

            uint8_t * p = packet;
            ++p; // skip the header

            switch (i) {
                case 0:
                case 1:
                    memcpy(sts->pubOther + i*16, p, 16);
                    break;
                case 2:
                    memcpy(sts->pubOther + i*16, p, 16);
                    sts_init(sts);
                    ecdh_shared_secret(sts->prv, sts->pubOther, sts->s);
                    return true;
                default:
                    return false;
            }

            ++i;
        }

    }

    return false;
}


/* ================================================ STS2 ================================================ */


bool
sts2_tx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    // pub | nonce | E_s(MAC_k(pub | pubOther))
    uint8_t packet[PAYLOADSIZE];

    // Send pub key
    radio->pack(packet, HEADER_STS2, sts->pub+0*16, 16);
    radio->send_packet(packet);
    radio->pack(packet, HEADER_STS2, sts->pub+1*16, 16);
    radio->send_packet(packet);
    radio->pack(packet, HEADER_STS2, sts->pub+2*16, 16);
    radio->send_packet(packet);

    // Generate and transmit nonce
    uint8_t temp[AES_BLOCKLEN];
    random_bytes(sts->iv, 8); // top half nonce
    memset(sts->iv+8, 0, 8); // bottom half zeros
    memcpy(temp, sts->iv, 8);
    pkcs_pad(temp, AES_BLOCKLEN, 8);
    radio->pack(packet, HEADER_STS2, temp, 16);
    radio->send_packet(packet);

    // MAC_k(pub | pubOther)
    uint8_t buf[2*ECC_PUB_KEY_SIZE];
    uint8_t mac[AES_BLOCKLEN];
    memcpy(buf, sts->pub, ECC_PUB_KEY_SIZE);
    memcpy(buf+ECC_PUB_KEY_SIZE, sts->pubOther, ECC_PUB_KEY_SIZE);
    cbc_mac(mac, buf, sizeof(buf), sts->k);
    printf("<%s> mac: ", __func__); print_array(mac, AES_BLOCKLEN);
    
    // E_s(MAC_k(pub | pubOther)
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
    AES_CTR_xcrypt_buffer(&ctx, mac, AES_BLOCKLEN); // mac now contains the encrypted mac

    // Transmit E_s(MAC_k(pub | pubOther)
    radio->pack(packet, HEADER_STS2, mac, 16);
    radio->send_packet(packet);    

    radio->startListening();
    return true;
}


bool
sts2_rx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    // pubB | nonce | E_s(MAC_k(pubOther | pub)) -->  80/16 = 5 packets (ignoring headers)
    // i=0,1,2  pubB            16, 16, 16
    // i=3      nonce | PAD8    8, 8
    // i=4      E_s             16
    uint8_t packet[PAYLOADSIZE];
    uint8_t pipe;
    uint8_t header;

    uint32_t t = time_us_32();
    int i = 0;
    while (((time_us_32() - t) / 1000) < STS2_RX_TIMEOUT_MS) {
        if (radio->receive_packet(packet, &pipe)) {
            // TODO: if an intermediate packet is received -> reset t = time_us_32();
            // TODO: send i with the packet? --> allows for retransmissions / bursts without much trouble: only process i if it hasn't been already
            //          This could also be achieved with more headers, one for every specific packet STS1_1, STS1_2, ...
            header = *packet;

            if (header != HEADER_STS2) {
                printf("Incorrect header in %s: %d\n", __func__, header);
                return false;
            }

            uint8_t * p = packet;
            ++p; // skip the header

            switch (i) {
                case 0:
                case 1:
                    memcpy(sts->pubOther + i*16, p, 16);
                    break;
                case 2:
                    memcpy(sts->pubOther + i*16, p, 16);
                    ecdh_shared_secret(sts->prv, sts->pubOther, sts->s);
                    break;
                case 3:
                    // nonce
                    if (!pkcs_verify(p, AES_BLOCKLEN, 8))
                        return false;
                    memcpy(sts->iv, p, 8);
                    memset(sts->iv+8, 0, 8);
                    break;
                case 4: {
                    // E_s
                    // p contains the cipthertext E_s(MAC_k(pubOther | pub))
                    AES_ctx ctx;
                    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
                    AES_CTR_xcrypt_buffer(&ctx, p, AES_BLOCKLEN); // p now contains the reference MAC
                    printf("<%s> rcvd mac: ", __func__); print_array(p, AES_BLOCKLEN);

                    // Compute the mac ourselves and compare
                    uint8_t buf[2*ECC_PUB_KEY_SIZE];
                    uint8_t mac[AES_BLOCKLEN];
                    memcpy(buf, sts->pubOther, ECC_PUB_KEY_SIZE);
                    memcpy(buf+ECC_PUB_KEY_SIZE, sts->pub, ECC_PUB_KEY_SIZE);
                    cbc_mac(mac, buf, sizeof(buf), sts->k);
                    printf("<%s> computed mac: ", __func__); print_array(mac, AES_BLOCKLEN);

                    return memcmp(mac, p, AES_BLOCKLEN) == 0;
                }
                default:
                    return false;
            }

            ++i;
        }
    }

    return false;
}


/* ================================================ STS3 ================================================ */


bool
sts3_tx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    // E_s(MAC_k(pub | pubOther)) **16 bytes** ctr = 1

    // Compute the mac
    uint8_t buf[2*ECC_PUB_KEY_SIZE];
    uint8_t mac[AES_BLOCKLEN];
    memcpy(buf, sts->pub, ECC_PUB_KEY_SIZE);
    memcpy(buf+ECC_PUB_KEY_SIZE, sts->pubOther, ECC_PUB_KEY_SIZE);
    cbc_mac(mac, buf, sizeof(buf), sts->k);

    // Encrypt the MAC
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
    ctx.Iv[AES_BLOCKLEN-1] += 1;
    AES_CTR_xcrypt_buffer(&ctx, mac, AES_BLOCKLEN);

    // Transmit
    uint8_t packet[PAYLOADSIZE];
    radio->pack(packet, HEADER_STS3, mac, 16);
    radio->send_packet(packet);
    radio->startListening();
    return true;
}


bool
sts3_rx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    // E_s(MAC_k(pubOther | pub)) **16 bytes** ctr = 1
    uint8_t packet[PAYLOADSIZE];
    uint8_t pipe;
    uint8_t header;

    uint32_t t = time_us_32();
    int i = 0;
    while (((time_us_32() - t) / 1000) < STS3_RX_TIMEOUT_MS) {
        if (radio->receive_packet(packet, &pipe)) {
            header = *packet;

            if (header != HEADER_STS3) {
                printf("Incorrect header in %s: %d\n", __func__, header);
                return false;
            }

            uint8_t * p = packet;
            ++p; // skip the header

            AES_ctx ctx;
            AES_init_ctx_iv(&ctx, sts->s, sts->iv);
            ctx.Iv[AES_BLOCKLEN-1] += 1;
            AES_CTR_xcrypt_buffer(&ctx, p, AES_BLOCKLEN); // p now contains the reference MAC
            printf("<%s> rcvd mac: ", __func__); print_array(p, AES_BLOCKLEN);

            // Compute the mac ourselves and compare
            uint8_t buf[2*ECC_PUB_KEY_SIZE];
            uint8_t mac[AES_BLOCKLEN];
            memcpy(buf, sts->pubOther, ECC_PUB_KEY_SIZE);
            memcpy(buf+ECC_PUB_KEY_SIZE, sts->pub, ECC_PUB_KEY_SIZE);
            cbc_mac(mac, buf, sizeof(buf), sts->k);
            printf("<%s> computed mac: ", __func__); print_array(mac, AES_BLOCKLEN);

            return memcmp(mac, p, AES_BLOCKLEN) == 0;
        }
    }
    
    return false;
}


/* ================================================ CMD ================================================ */
bool
cmd_tx(Radio * radio, struct STS * sts, uint8_t command)
{
    printf("<%s>\n", __func__);

    // E_s(CMD)
    uint8_t buf[AES_BLOCKLEN];
    *buf = command;
    pkcs_pad(buf, sizeof(buf), sizeof(*buf));

    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
    ctx.Iv[AES_BLOCKLEN-1] += 2;
    AES_CTR_xcrypt_buffer(&ctx, buf, AES_BLOCKLEN);

    uint8_t packet[PAYLOADSIZE];
    radio->pack(packet, HEADER_CMD, buf, 16);
    radio->send_packet(packet);

    // MAC_s(E_s(CMD))
    uint8_t mac[AES_BLOCKLEN];
    cbc_mac(mac, buf, sizeof(buf), sts->s);

    radio->pack(packet, HEADER_CMD, mac, 16);
    radio->send_packet(packet);

    radio->startListening();

    return true;
}


bool
cmd_rx(Radio * radio, struct STS * sts, uint8_t * command)
{
    printf("<%s>\n", __func__);

    // E_s(CMD) | MAC_s(E_s(CMD)) **32 bytes** ctr = 2
    // i=0      E_s(CMD)            16
    // i=1      MAC_s(E_s(CMD))     16
    uint8_t packet[PAYLOADSIZE];
    uint8_t pipe;
    uint8_t header;
    uint8_t ciphertext[AES_BLOCKLEN];

    uint32_t t = time_us_32();
    int i = 0;
    while (((time_us_32() - t) / 1000) < CMD_RX_TIMEOUT_MS) {
        if (radio->receive_packet(packet, &pipe)) {
            header = *packet;

            if (header != HEADER_CMD) {
                printf("Incorrect header in %s: %d\n", __func__, header);
                return false;
            }

            uint8_t * p = packet;
            ++p; // skip the header

            switch (i) {
                case 0: { // E_s(CMD)
                    AES_ctx ctx;
                    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
                    ctx.Iv[AES_BLOCKLEN-1] += 2;
                    memcpy(ciphertext, p, AES_BLOCKLEN);
                    AES_CTR_xcrypt_buffer(&ctx, p, AES_BLOCKLEN);
                    if (!pkcs_verify(p, 1, AES_BLOCKLEN-1))
                        return false;
                    printf("<%s> rcvd command: ", __func__); print_array(p, AES_BLOCKLEN);
                    *command = *p;
                    break;
                }
                case 1: { // MAC_s(E_s(CMD))
                    // p points to the received MAC_s(E_s(CMD))
                    // compute MAC_s(ciphertext) and compare
                    uint8_t mac[AES_BLOCKLEN];
                    cbc_mac(mac, ciphertext, sizeof(ciphertext), sts->s);
                    printf("<%s> rcvd mac: ", __func__); print_array(p, AES_BLOCKLEN);
                    printf("<%s> computed mac: ", __func__); print_array(mac, AES_BLOCKLEN);

                    return memcmp(mac, p, AES_BLOCKLEN) == 0;
                }
                default:
                    return false;
            }

            ++i;
        }
    }

    return false;
}

/* ================================================ ACK ================================================ */
bool
ack_tx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    // E_s(ACK)
    uint8_t buf[AES_BLOCKLEN];
    *buf = BYTE_ACK;
    pkcs_pad(buf, sizeof(buf), sizeof(*buf));

    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
    ctx.Iv[AES_BLOCKLEN-1] += 3;
    AES_CTR_xcrypt_buffer(&ctx, buf, AES_BLOCKLEN);

    uint8_t packet[PAYLOADSIZE];
    radio->pack(packet, HEADER_ACK, buf, 16);
    radio->send_packet(packet);

    // MAC_s(E_s(ACK))
    uint8_t mac[AES_BLOCKLEN];
    cbc_mac(mac, buf, sizeof(buf), sts->s);

    radio->pack(packet, HEADER_ACK, mac, 16);
    radio->send_packet(packet);

    radio->startListening();

    return true;
}


bool
ack_rx(Radio * radio, struct STS * sts)
{
    printf("<%s>\n", __func__);

    // E_s(ACK) | MAC_s(E_s(ACK)) **32 bytes** ctr = 3
    // i=0      E_s(ACK)            16
    // i=1      MAC_s(E_s(ACK))     16
    uint8_t packet[PAYLOADSIZE];
    uint8_t pipe;
    uint8_t header;
    uint8_t ciphertext[AES_BLOCKLEN];

    uint32_t t = time_us_32();
    int i = 0;
    while (((time_us_32() - t) / 1000) < ACK_RX_TIMEOUT_MS) {
        if (radio->receive_packet(packet, &pipe)) {
            header = *packet;

            if (header != HEADER_ACK) {
                printf("Incorrect header in %s: %d\n", __func__, header);
                return false;
            }

            uint8_t * p = packet;
            ++p; // skip the header

            switch (i) {
                case 0: { // E_s(ACK)
                    AES_ctx ctx;
                    AES_init_ctx_iv(&ctx, sts->s, sts->iv);
                    ctx.Iv[AES_BLOCKLEN-1] += 3;
                    memcpy(ciphertext, p, AES_BLOCKLEN);
                    AES_CTR_xcrypt_buffer(&ctx, p, AES_BLOCKLEN);
                    if (!pkcs_verify(p, 1, AES_BLOCKLEN-1))
                        return false;
                    printf("<%s> rcvd ack: ", __func__); print_array(p, AES_BLOCKLEN);
                    if (*p != BYTE_ACK)
                        return false;
                    break;
                }
                case 1: { // MAC_s(E_s(ACK))
                    // p points to the received MAC_s(E_s(ACK))
                    // compute MAC_s(ciphertext) and compare
                    uint8_t mac[AES_BLOCKLEN];
                    cbc_mac(mac, ciphertext, sizeof(ciphertext), sts->s);
                    printf("<%s> rcvd mac: ", __func__); print_array(p, AES_BLOCKLEN);
                    printf("<%s> computed mac: ", __func__); print_array(mac, AES_BLOCKLEN);

                    return memcmp(mac, p, AES_BLOCKLEN) == 0;
                }
                default:
                    return false;
            }

            ++i;
        }
    }
    
    return false;
}
