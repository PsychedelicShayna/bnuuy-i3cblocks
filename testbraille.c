#include "braille.h"
#include <locale.h>
#include <stdio.h>
#include <wchar.h>

int main() {
    setlocale(LC_ALL, "");
    // Test data
    // uint8_t test_data[] = {0x00, 0xFF, 0x7F, 0x80, 0x55, 0xAA};
    uint8_t test_data[13]    = { 1, 2, 4, 8, 16, 32, 48, 64, 96, 128, 196, 244, 255 };
    size_t  test_data_size = sizeof(test_data) / sizeof(test_data[0]);

    // Buffer to hold braille output
    wchar_t braille_output[256];

    // Convert to braille
    size_t bytes_written = braille_write(
      braille_output, sizeof(braille_output), test_data, sizeof(test_data));

    // Print results
    wprintf(L"Braille Output:\n");
    for(size_t i = 0; i < bytes_written; ++i) {
        wprintf(L"%lc ", braille_output[i]);
    }
    wprintf(L"\nBytes Written: %zu\n", bytes_written);

    return 0;
}





























































