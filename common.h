#ifndef _COMMON_H
#define _COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void urandom(uint8_t* out, size_t szout)
{
    FILE* fd = fopen("/dev/urandom", "r");
    fread(out, 1, szout, fd);
    fclose(fd);
}

bool coinflip(void)
{
    uint8_t buf[1];
    urandom(buf, 1);

    return buf[0] > 127;
}

void hexdump(void* data, size_t size)
{
    for(size_t i = 0; i < size; ++i) {
        printf("%02lx", (uint64_t)(*((uint8_t*)data + i)));
    }
    printf("\n");
}


#endif
