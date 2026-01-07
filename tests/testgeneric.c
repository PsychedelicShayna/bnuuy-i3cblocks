#include <alloca.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define iprint(id, a, s)                                               \
    for(int i = 0; i < s; ++i) printf(" #%d#|%d|#%d# ", id, a[i], id); \
    printf("\n------------------\n");

void test(unsigned short* data, size_t sz)
{
    static unsigned short* memory = NULL;

    if(!memory) {
        memory = malloc(sz * 2);
        memcpy(memory, data, sizeof(unsigned short) * sz);
        iprint(4, memory, sz);
    } else {
        iprint(5, data, sz);
        iprint(6, memory, sz);
    }
}

int main(void)
{

    unsigned short data[16];
    for(int i = 0; i < 16; ++i) {
        uint8_t  buf[sizeof(short)];
        uint8_t* bufp = buf;

        urandom(bufp, sizeof(short));
        data[i] = *(short*)bufp;
        // printf("%d, ", data[i]);
    }

    // printf("\n------------------\n");

    iprint(1, data, 16);
    test(data, 16);

    for(int i = 0; i < 16; ++i) {
        uint8_t  buf[sizeof(short)];
        uint8_t* bufp = buf;

        urandom(bufp, sizeof(short));
        data[i] = *(short*)bufp;
        // printf("%d, ", data[i]);
    }

    iprint(2, data, 16);
    test(data, 16);
}
