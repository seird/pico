#ifndef __UTILS_H__
#define __UTILS_H__


#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


void print_array(uint8_t * a, size_t size);
/**
 * @brief 
 * 
 * @param b 
 * @param size MUST be a multiple of 8
 */
void random_bytes(uint8_t * b, size_t size);
void pkcs_pad(uint8_t * b, size_t size, size_t clear_text_size);
bool pkcs_verify(uint8_t * b, size_t size, size_t clear_text_size);


#ifdef __cplusplus
}
#endif


#endif // __UTILS_H__
