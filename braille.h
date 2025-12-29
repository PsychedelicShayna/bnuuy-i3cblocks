#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

// clang-format off
const wchar_t BRAILLE_TABLE[257] = {
    /* ____0123456789ABCDEF*/
    /*0*/L"⠀⠁⠂⠃⠄⠅⠆⠇⡀⡁⡂⡃⡄⡅⡆⡇"
    /*1*/ "⠈⠉⠊⠋⠌⠍⠎⠏⡈⡉⡊⡋⡌⡍⡎⡏"
    /*2*/ "⠐⠑⠒⠓⠔⠕⠖⠗⡐⡑⡒⡓⡔⡕⡖⡗"
    /*3*/ "⠘⠙⠚⠛⠜⠝⠞⠟⡘⡙⡚⡛⡜⡝⡞⡟"
    /*4*/ "⠠⠡⠢⠣⠤⠥⠦⠧⡠⡡⡢⡣⡤⡥⡦⡧"
    /*5*/ "⠨⠩⠪⠫⠬⠭⠮⠯⡨⡩⡪⡫⡬⡭⡮⡯"
    /*6*/ "⠰⠱⠲⠳⠴⠵⠶⠷⡰⡱⡲⡳⡴⡵⡶⡷"
    /*7*/ "⠸⠹⠺⠻⠼⠽⠾⠿⡸⡹⡺⡻⡼⡽⡾⡿"
    /*8*/ "⢀⢁⢂⢃⢄⢅⢆⢇⣀⣁⣂⣃⣄⣅⣆⣇"
    /*9*/ "⢈⢉⢊⢋⢌⢍⢎⢏⣈⣉⣊⣋⣌⣍⣎⣏"
    /*A*/ "⢐⢑⢒⢓⢔⢕⢖⢗⣐⣑⣒⣓⣔⣕⣖⣗"
    /*B*/ "⢘⢙⢚⢛⢜⢝⢞⢟⣘⣙⣚⣛⣜⣝⣞⣟"
    /*C*/ "⢠⢡⢢⢣⢤⢥⢦⢧⣠⣡⣢⣣⣤⣥⣦⣧"
    /*D*/ "⢨⢩⢪⢫⢬⢭⢮⢯⣨⣩⣪⣫⣬⣭⣮⣯"
    /*E*/ "⢰⢱⢲⢳⢴⢵⢶⢷⣰⣱⣲⣳⣴⣵⣶⣷"
    /*F*/ "⢸⢹⢺⢻⢼⢽⢾⢿⣸⣹⣺⣻⣼⣽⣾⣿"
};

/* Read up to sz_src bytes from src and write the braille representation as a
 * wchar_t to dst; this function asserts that sz_dst is less than sz_src.
 *
 * sz_dst is not the number of bytes, but the number of wchar_t's that can be
 * written to dst. the sizeof is calculated internally.
 *
 * Returns the number of wchar_t's written to dst, multily it by sizeof(wchar_t)
 * to get the number of bytes written.
 * */
static inline size_t braille_write(wchar_t* dst, size_t sz_dst,
                                   uint8_t* src, size_t sz_src)
{ /* clang-format on */

    size_t index     = 0;

    for(; index < sz_dst; ++index) {

        if(index >= sz_src) {
            return index;
        }

        wchar_t byte_as_braille = BRAILLE_TABLE[src[index]];
        dst[index]              = byte_as_braille;
    }

    return index;
}
















