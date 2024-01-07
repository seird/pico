#include "utils/utils.h"
#include <pico/rand.h>
#include <stdio.h>
#include <string.h>


void
print_array(uint8_t * a, size_t size)
{
    for (size_t i=0; i<size; ++i)
        printf("%02x", *(a+i));
    putchar('\n');
}


void
random_bytes(uint8_t * b, size_t size)
{
    for (size_t i=0; i<size; i+=sizeof(uint64_t)) {
        *(uint64_t *)(b + i) = get_rand_64();
    }
}


// https://datatracker.ietf.org/doc/html/rfc5652#section-6.3
void
pkcs_pad(uint8_t * b, size_t size, size_t clear_text_size)
{
    memset(b+clear_text_size, size-clear_text_size, size-clear_text_size);
}


bool
pkcs_verify(uint8_t * b, size_t size, size_t clear_text_size)
{
    for (int i=clear_text_size; i<size; ++i)
        if (b[i] != (size-clear_text_size))
            return false;

    return true;
}
