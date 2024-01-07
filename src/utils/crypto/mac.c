#include "utils/aes.h"
#include "utils/mac.h"
#include <string.h>


void
cbc_mac(uint8_t mac[AES_BLOCKLEN], uint8_t * data, size_t size, uint8_t * key)
{
    uint8_t IV[AES_BLOCKLEN];
    memset(IV, 0, AES_BLOCKLEN);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, IV);
    AES_CBC_encrypt_buffer(&ctx, data, size);

    memcpy(mac, data + size-AES_BLOCKLEN, AES_BLOCKLEN);
}
