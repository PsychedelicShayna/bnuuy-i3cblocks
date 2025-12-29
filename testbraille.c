#include "braille.h"
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

#define USLEEPFOR 10000
#include "cpu.h"

void cpu_usage_braille(void) {
    double history[64];
    size_t idx = 0;

    while(1) {
        double usage = cpu_usage();
        history[idx] = usage;
        if(idx >= sizeof(history) / sizeof(history[0]) - 1) {
            memcpy(history, &history[1],  (sizeof(history) / sizeof(history[0]) - 1) * sizeof(history[0]));
        } else {
            idx++;
        }

        wchar_t braille_chart[32];
        size_t  written =
          braille_inline_chart(braille_chart,
                               sizeof(braille_chart) / sizeof(braille_chart[0]),
                               history,
                               sizeof(history) / sizeof(history[0]), 0, 100);

        wprintf(L"CPU Usage: [");
        for(size_t i = 0; i < written; ++i) {
            wprintf(L"%lc", braille_chart[i]);
        }
        wprintf(L"] \r");
        fflush(stdout);

        // wprintf(L"\n");
    }
}

int main() {
    setlocale(LC_ALL, "");
    cpu_usage_braille();
    return 0;

    test_braille_chart();
    return 0;

    // 8 & 128
    // 8 & 4 & 128
    // 8 & 2 & 4 & 128
    // 8 & 1 & 2 & 4 & 128
    // 8 & 128 & 4 & 64
    // 8 & 128 & 4 & 64 & 32 & 2
    // 8 & 128 & 4 & 64 & 32 & 2 & 1 & 16
    // 8 & 16 & 32 & 64 & 128
    // 8 & 32 & 64 & 128
    // 8 & 64 & 128
    // 8 & 128

    uint8_t values[] = {

        8 | 128 | 64 |

          // right 4: left spectrum
          8 | 128 | 64 | 32 | 16,
        8 | 128 | 64 | 32 | 16 | 4,
        8 | 128 | 64 | 32 | 16 | 4 | 2,
        8 | 128 | 64 | 32 | 16 | 4 | 2 | 1,

        // right 3: left spectrum
        8 | 128 | 64 | 32,
        8 | 128 | 64 | 32 | 4,
        8 | 128 | 64 | 32 | 4 | 2,
        8 | 128 | 64 | 32 | 4 | 2 | 1,

        // right 2: left spectrum
        8 | 128 | 64,
        8 | 128 | 64 | 4,
        8 | 128 | 64 | 4 | 2,
        8 | 128 | 64 | 4 | 2 | 1,

        // right 1: left spectrum
        8 | 128,
        8 | 128 | 4,
        8 | 128 | 4 | 2,
        8 | 128 | 4 | 2 | 1,

        // left 4: right spectrum
        8 | 128 | 4 | 2 | 1,
        8 | 128 | 4 | 2 | 1 | 64,
        8 | 128 | 4 | 2 | 1 | 64 | 32,
        8 | 128 | 4 | 2 | 1 | 64 | 32 | 16,

        // left 3: right spectrum
        8 | 128 | 4 | 2,
        8 | 128 | 4 | 2 | 64,
        8 | 128 | 4 | 2 | 64 | 32,
        8 | 128 | 4 | 2 | 64 | 32 | 16,

        // left 2: right spectrum
        8 | 128 | 4,
        8 | 128 | 4 | 64,
        8 | 128 | 4 | 64 | 32,
        8 | 128 | 4 | 64 | 32 | 16,

        // left 1: right spectrum
        8 | 128,
        8 | 128 | 64,
        8 | 128 | 64 | 32,
        8 | 128 | 64 | 32 | 16,

        // left & right 4
        8 | 128 | 64 | 32 | 16 | 4 | 2 | 1,
        // left & right 3
        8 | 128 | 64 | 32 | 4 | 2,
        // left & right 2
        8 | 128 | 64 | 4,
        // left & right 1
        8 | 128,

    };

    //   ⣀ ⣠⣤ ⣄ ⣰⣸
    // ⣄
    // ⣆
    // ⣇
    // ⣤
    // ⣶
    // ⣿
    // ⣸
    // ⣰
    // ⣠
    // ⣀

#ifdef INTERACTIVE

    char input[256];
    while(1) {
        memset(input, 0, 256);

        // Read input line
        printf("Combine RN with LN or q to exit\n");
        if(!fgets(input, sizeof(input), stdin)) {
            break; // Exit on input error
        }

        if(input[0] == 'q') {
            break; // Exit on 'q'
        }

        unsigned char value = 0;
        size_t        sl    = strlen(input);

        for(int i = 0; i < sl; ++i) {
            char c  = input[i];
            char nc = input[i + 1 < sl ? i + 1 : i];

            if(c == 'r' && nc == '1') {
                value |= R1;
                i++;
            } else if(c == 'r' && nc == '2') {
                value |= R2;
                i++;
            } else if(c == 'r' && nc == '3') {
                value |= R3;
                i++;
            } else if(c == 'r' && nc == '4') {
                value |= R4;
                i++;
            } else if(c == 'l' && nc == '1') {
                value |= L1;
                i++;
            } else if(c == 'l' && nc == '2') {
                value |= L2;
                i++;
            } else if(c == 'l' && nc == '3') {
                value |= L3;
                i++;
            } else if(c == 'l' && nc == '4') {
                value |= L4;
                i++;
            }
        }

        wchar_t braille = BRAILLE_TABLE[value];

        printf("\nValue: %d, Braille: %lc\n", value, braille);
    }

#endif
    //

    for(int i = 0; i < sizeof(values); i++) {
        wchar_t c = BRAILLE_TABLE[values[i]];
        wprintf(L"%d: %lc\n", i, c);
        // if(i & 8 & 127) {
        //     wchar_t c = BRAILLE_TABLE[i];
        //     wprintf(L"%d: %lc\n", i, c);
        // }
    }

    // // Test data
    // // uint8_t test_data[] = {0x00, 0xFF, 0x7F, 0x80, 0x55, 0xAA};
    // uint8_t test_data[13]    = { 1, 2, 4, 8, 16, 32, 48, 64, 96, 128, 196,
    // 244, 255 }; size_t  test_data_size = sizeof(test_data) /
    // sizeof(test_data[0]);
    //
    // // Buffer to hold braille output
    // wchar_t braille_output[256];
    //
    // // Convert to braille
    // size_t bytes_written = braille_write(
    //   braille_output, sizeof(braille_output), test_data, sizeof(test_data));
    //
    // // Print results
    // wprintf(L"Braille Output:\n");
    // for(size_t i = 0; i < bytes_written; ++i) {
    //     wprintf(L"%lc ", braille_output[i]);
    // }
    // wprintf(L"\nBytes Written: %zu\n", bytes_written);

    return 0;
}
