#ifndef __MAC_H__
#define __MAC_H__


#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Compute CBC-MAC for data. Data size must be a multiple of AES_BLOCKLEN.
 * 
 * @param mac 
 * @param data 
 * @param size 
 * @param key 
 */
void cbc_mac(uint8_t * mac, uint8_t * data, size_t size, uint8_t * key);


#ifdef __cplusplus
}
#endif


#endif // __MAC_H__
