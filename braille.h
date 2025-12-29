#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

//       ⣦   ⢰
//    ⣦
//
// left 1          ⣰
// right 1
// left 2          ⣤ ⣴ ⣼
// right 2       ⣠ ⣤ ⣦ ⣧
// left 3        ⣆ ⣦ ⣶ ⣾
// right 3       ⣰ ⣴ ⣶ ⣷
// left 4        ⣇ ⣧ ⣷ ⣿
// right 4       ⣸ ⣼ ⣾ ⣿
// equal         ⣿ ⣶ ⣤ ⣀

// const wchar_t
//     c11 = L'⣀',
//     c12 = L'⣠',
//     c13 = L'⣰',
//     c14 = L'⣸',
//     c21 =

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


// For chart
#define R4 8 | 128 | 64 | 32 | 16
#define R3 8 | 128 | 64 | 32
#define R2 8 | 128 | 64
#define R1 8 | 128
#define L1 8 | 128
#define L2 8 | 128 | 4
#define L3 8 | 128 | 4 | 2
#define L4 8 | 128 | 4 | 2 | 1

// clang-format on

/*


const wchar_t BRAILLE_TABLE[257] = {
  L"⠀⠁⠂⠃⠄⠅⠆⠇⡀⡁⡂⡃⡄⡅⡆⡇"
   "⠈⠉⠊⠋⠌⠍⠎⠏⡈⡉⡊⡋⡌⡍⡎⡏"
   "⠐⠑⠒⠓⠔⠕⠖⠗⡐⡑⡒⡓⡔⡕⡖⡗"
   "⠘⠙⠚⠛⠜⠝⠞⠟⡘⡙⡚⡛⡜⡝⡞⡟"
   "⠠⠡⠢⠣⠤⠥⠦⠧⡠⡡⡢⡣⡤⡥⡦⡧"
   "⠨⠩⠪⠫⠬⠭⠮⠯⡨⡩⡪⡫⡬⡭⡮⡯"
   "⠰⠱⠲⠳⠴⠵⠶⠷⡰⡱⡲⡳⡴⡵⡶⡷"
   "⠸⠹⠺⠻⠼⠽⠾⠿⡸⡹⡺⡻⡼⡽⡾⡿"
   "⢀⢁⢂⢃⢄⢅⢆⢇⣀⣁⣂⣃⣄⣅⣆⣇"
   "⢈⢉⢊⢋⢌⢍⢎⢏⣈⣉⣊⣋⣌⣍⣎⣏"
   "⢐⢑⢒⢓⢔⢕⢖⢗⣐⣑⣒⣓⣔⣕⣖⣗"
   "⢘⢙⢚⢛⢜⢝⢞⢟⣘⣙⣚⣛⣜⣝⣞⣟"
   "⢠⢡⢢⢣⢤⢥⢦⢧⣠⣡⣢⣣⣤⣥⣦⣧"
   "⢨⢩⢪⢫⢬⢭⢮⢯⣨⣩⣪⣫⣬⣭⣮⣯"
   "⢰⢱⢲⢳⢴⢵⢶⢷⣰⣱⣲⣳⣴⣵⣶⣷"
   "⢸⢹⢺⢻⢼⢽⢾⢿⣸⣹⣺⣻⣼⣽⣾⣿"
};

setlocale(LC_ALL, "')
for(int i=0; i<256; i++) {
wchar_t c = BRAILLE_TABLE[i];
printf("%lc\n", c);
}
   */

// clang-format off
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

    size_t index = 0;

    for(; index < sz_dst; ++index) {

        if(index >= sz_src) {
            return index;
        }

        wchar_t byte_as_braille = BRAILLE_TABLE[src[index]];
        dst[index]              = byte_as_braille;
    }

    return index;
}

static inline void
  minmaxf(double* array, size_t sz_array, double** outmin, double** outmax) {
    double *min = array, *max = array;

    for(size_t i = 0; i < sz_array; ++i) {
        if(array[i] > *max) {
            max = &array[i];
        } else if(array[i] < *min) {
            min = &array[i];
        }
    }

    *outmin = min;
    *outmax = max;
}

static inline uint8_t classify(double v1, double v2) {
    uint8_t c = 0;
    if(v1 < 25)
        c |= L1;
    else if(v1 < 50)
        c |= L2;
    else if(v1 < 75)
        c |= L3;
    else if(v1 >= 75)
        c |= L4;

    if(v2 < 25)
        c |= R1;
    else if(v2 < 50)
        c |= R2;
    else if(v2 < 75)
        c |= R3;
    else if(v2 >= 75)
        c |= R4;

    return c;
}

/* clang-format off */
static inline size_t braille_inline_chart(wchar_t* dst, size_t sz_dst,
                                          double*  data, size_t sz_data)
{ /* clang-format on */

    wchar_t chart[sz_dst];
    size_t  idx = 0;

    double *min, *max;
    minmaxf(data, sz_data, &min, &max);

    for(size_t i = 0; i < sz_data; i++) {
        double v1 = data[i];
        double v2 = data[i + 1 < sz_data ? i + 1 : i];

        double norm1 = (v1 - *min) / (*max - *min);
        double perc1 = norm1 * 100;

        double norm2 = (v2 - *min) / (*max - *min);
        double perc2 = norm2 * 100;

        uint8_t vcode   = classify(v1, v2);
        wchar_t braille = BRAILLE_TABLE[vcode];
        chart[idx++]    = braille;

        if(idx >= sz_dst) {
            break;
        }

        i++;
    }

    memcpy(dst, chart, sz_dst * sizeof(wchar_t));
    return idx;
}


void test_braille_chart(void) {
    size_t ds     = 10;
    double data[] = { 13.0,   15.124,  50.1234, 19.123, 6.124,
                      62.234, 10000.0, 5000.0,  300.0,  700.0 };

    wchar_t chart[ds];
    size_t  written = braille_inline_chart(chart, ds, data, ds);

    for(size_t i = 0; i < written; ++i) {
        wprintf(L"%lc", chart[i]);
        fflush(stdout);
    }

    wprintf(L"\n");
}
/*

void minmax(double* array, size_t sz_array, double** outmin, double**
outmax) { double *min = array, *max = array;

    for(size_t i = 0; i < sz_array; ++i) {
        if(array[i] > *max) {
            max = &array[i];
        } else if(array[i] < *min) {
            min = &array[i];
        }
    }

    *outmin = min;
    *outmax = max;
}

size_t ds = 7;
   double data[] = { 13.0, 15.124, 50.1234, 19.123, 6.124, 62.234, 10000.0
};

   double *min, *max;
   minmax(data, ds, &min, &max);

   for(int i =0; i<ds;++i) {
   double value = data[i];
        double normalized = (value - *min) / (*max - *min);
    printf("%lf %lf %lf %lf\n", data[i], *min,*max, data[i] / *max);
    printf("Mapped: %lf %lf\n", 4.0 / (data[i] / *max)*100, 4.0 / (data[i] /
*max))  ; printf("Norm :%lf\n", (normalized*100.0)/4.0);
   }







 */
