#ifndef __STS_H__
#define __STS_H__


#include "utils/radio.hpp"
#include "utils/aes.h"
#include "utils/ecdh.h"
#include <stdint.h>
#include <stdbool.h>


struct STS {
    uint8_t pub[ECC_PUB_KEY_SIZE];
    uint8_t prv[ECC_PRV_KEY_SIZE];
    uint8_t s[ECC_PUB_KEY_SIZE];
    uint8_t pubOther[ECC_PUB_KEY_SIZE];
    uint8_t iv[AES_BLOCKLEN];
    uint8_t k[AES_BLOCKLEN]; // Pre-shared key for authentication
};


bool sts_init(struct STS * sts);
void sts_set_key(struct STS * sts, const uint8_t * k);

bool sts1_tx(Radio * radio, struct STS * sts);
bool sts1_rx(Radio * radio, struct STS * sts);

bool sts2_tx(Radio * radio, struct STS * sts);
bool sts2_rx(Radio * radio, struct STS * sts);

bool sts3_tx(Radio * radio, struct STS * sts);
bool sts3_rx(Radio * radio, struct STS * sts);

bool cmd_tx(Radio * radio, struct STS * sts, uint8_t command);
bool cmd_rx(Radio * radio, struct STS * sts, uint8_t * command);

bool ack_tx(Radio * radio, struct STS * sts);
bool ack_rx(Radio * radio, struct STS * sts);


#endif // __STS_H__
