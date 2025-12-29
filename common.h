
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void hexdump(void* data, size_t size) {
    for(size_t i = 0; i < size; ++i) {
        printf("%02lx", (uint64_t)(*((uint8_t*)data + i)));
    }
    printf("\n");
}
