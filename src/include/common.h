#ifndef _COMMON_H
#define _COMMON_H

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include <math.h>

#ifdef JSON_MACROS

#define jget(type, obj, key)                         \
    ({                                               \
        struct json_object* _obj;                    \
        json_object_object_get_ex(obj, #key, &_obj); \
        json_object_get_##type(_obj);                \
    })

#define jgeto(obj, key)                              \
    ({                                               \
        struct json_object* _obj;                    \
        json_object_object_get_ex(obj, #key, &_obj); \
        _obj;                                        \
    })
#endif


#ifdef DEBUG
#define debug(message)                  \
    {                                   \
        fprintf(stderr, "%s", message); \
        fflush(stderr);                 \
    }

#define debugf(format, ...)                   \
    {                                         \
        fprintf(stderr, format, __VA_ARGS__); \
        fflush(stderr);                       \
    }
#else
#define debug(message)
#define debugf(format, ...)
#endif

double truncate_precision(double x, int precision)
{
    double factor = pow(10.0, precision);
    return floor(x * factor) / factor;
}

typedef struct {
    double value;
    char   suffix;
    int    precision;
} human_size_t;

human_size_t human_size(uint64_t bytes)
{
    static const char suffixes[] = "BKMGTP";
    size_t            idx        = 0;

    double value = (double)bytes;

    while(idx < sizeof(suffixes) - 1) {
        if(value < 100.0) {

            if(idx == 0) {
                return (human_size_t) { value, suffixes[idx], 0 };
            }

            int precision = value < 10.0 ? 2 : 1;
            value         = truncate_precision(value, precision);
            return (human_size_t) { value, suffixes[idx], precision };
        }

        // value >= 100 not printable, force rescale
        value /= 1000.0;
        idx++;
    }

    // fallback (should not normally hit)
    value = truncate_precision(value, 2);
    return (human_size_t) { value, suffixes[idx], 2 };
}

double represent_size(double size, char* suffix)
{
    ulong suffix_idx = 0;
    char  suffixes[] = { 'B', 'K', 'M', 'G', 'T' };

    if(suffix != NULL) {
        switch(*suffix) {
            case 'B':
                suffix_idx = 0;
                break;
            case 'K':
                suffix_idx = 1;
                break;
            case 'M':
                suffix_idx = 2;
                break;
            case 'G':
                suffix_idx = 3;
                break;
            case 'T':
                suffix_idx = 4;
                break;
            default:
                suffix_idx = 0;
                break;
        }
    }

    while(size >= 1000) {
        size /= 1000;
        suffix_idx++;
    }

    if(suffix != NULL && suffix_idx < sizeof(suffixes)) {
        *suffix = suffixes[suffix_idx];
    }

    return size;
}

/* root [222] b(13);b(13+31);b(13+32);b(16+31);b(32);


```hy
  (print(list(map
      fn [N] (** n)
  )))

 (print (list (map bin (map (fn [x] (* x 32)) (range 32) ))))
```
*/

//
//
// 0000000000101100
// 0000000000101101
// 0000000000101111
// 0000000000100000

uintptr_t align(uintptr_t num, uintptr_t pow2)
{
    if((pow2 & (pow2 - 1)) != 0) {
        fprintf(stderr, "Not a pow2: %ld\n", pow2);
    }

    return ((num + pow2) - 1) & ~(uintptr_t)(pow2 - 1);
}

static inline bool ispow2(uintmax_t value)
{
    // Logically, a power of 2 minus 1 CANNOT have bit in common flipped.
    // i.e. 1000-1 = 0111 & 1000 == 0; this avoids division with % modulo.
    return value && ((value & (value - 1))) == 0;
}

/*
 let v = 13, align = 32

INVARIANTS:
  R % 32 == 0
  R must be multiple of 32, so R must be a value such that
  R + (32 * N) % 32 == 0
  (N)*32 ... (2):64 (3):96 (4):128 (5):160 (6):192 (7):224 (8):256
  N % 32 always non-zero if N < 32
  when we align-1 we're getting every bit that's not that pow2
  32-1 = 16,8,4,2,1
  0000000000001101
  */

// Return multiple of `pow2` that is greater or equal to `n`

void urandom(uint8_t* out, size_t szout)
{
    FILE* fd = fopen("/dev/urandom", "r");
    fread(out, 1, szout, fd);
    fclose(fd);
}

#define urandfloat(T, from, to)                         \
    ({                                                  \
        uint8_t buf[sizeof(T) + alignof(T)];            \
        urandom(buf, sizeof(buf));                      \
        T* t = (T*)alignup((uintptr_t)buf, alignof(T)); \
        *t   = from + ((*t / (T)(-1)) * (to - from));   \
        *t;                                             \
    })

#define urandint(T, from, to)      \
    ({                             \
        uint8_t buf[sizeof(T)];    \
        urandom(buf, sizeof(buf)); \
        T* t = (T*)buf;            \
        *t %= (to - from);         \
        *t += from;                \
        *t;                        \
    })

// #define urandint(T, from, to)
//     ((T)urandint(sizeof(T), (uintmax_t)(from), (uintmax_t)(to)))

//
// {
//     uint8_t buf[8];
//     urandom(buf, 8);
//     return *((uint64_t*)buf);
// }
//

// #define randint_generic(T, from, to)
//     ({
//         uint8_t buf[sizeof(T) + alignof(T)];
//         urandom(buf, sizeof(buf));
//         T* t = (T*)alignup((uintptr_t)buf, alignof(T));
//         *t %= (to - from);
//         *t += from;
//         *t;
//     })
//

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

void whexdump(void* data, size_t size)

{
    for(size_t i = 0; i < size; ++i) {
        wprintf(L"%02lx", (uint64_t)(*((uint8_t*)data + i)));
    }
    wprintf(L"\n");
}

// int safe_float_compare(float f1, float f2)
// {
//     // Ensure both floats are normalized to [0, 1]
//     float min = fmin(f1, f2);
//     float max = fmax(f1, f2);
//
//     // Calculate the number of bits needed to represent the difference
//     int bits =
//       8 * sizeof(float) - __builtin_clzll((uint64_t)((max - min) * (1 <<
//       23)));
//
//     // Compare the significant bits
//     return memcmp(&f1, &f2, bits / 8);
// }

#endif
