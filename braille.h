#include <alloca.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
size_t braille_write(wchar_t* dst, size_t sz_dst,
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

void minmaxf(double* array, size_t sz_array, double** outmin, double** outmax)
{
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

uint8_t classify(double v1, double v2)
{
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

double lerp(double v0, double v1, double t)
{
    return (1 - t) * v0 + t * v1;
}

// clang-format off

/* Writes a braille chart to `wchar_t* outchart` derived from the
 * doubles within `double* data`.
 *
 * `-`
 *
 * The `outlen` is the length, not the size, of `out` meaning it is the amount 
 * of `wchar_t` that `out` can hold and the same goes for `len` as the amount 
 * of `double` in `data`. 
 *
 * `-`
 *
 * The last `wchar_t` written to the `out` buffer will be a null terminator 
 * therefore if you want a chart with a length of 16 braille characters, you 
 * should make a `wchar_t[17]`.
 *
 * `-`
 *
 * The `dmin` and `dmax` doubles are fixed lower and upper bounds for the chart
 * that determine what the relative value of each double in `data` is between
 * the two bounds. If any of the two have a negative value then the bounds are
 * updated dynamically based on the minimum and maximum value in data. It is
 * reasonable to set `dmin` to `0.0` and `dmax` to `100.0` if you want the chart
 * to always represent an absolute percentage.
 *
 * */
size_t write_braille_chart(wchar_t* out, size_t outlen,
                           double* data, size_t len,
                           double  dmin, double dmax)
{ /* clang-format on */

    // Temperature; the "heat" of the chart, increased when values enter the
    // system, and diffuses slowly as they leave the system. Apparently this
    // idea is already a thing in audio? Peak detection with attack/release?
    // Discovering that isn't gonna make me conform to some existing algorithm
    // I'm calling it temperature! It makes sense! Like.. a hot red barrel of
    // a rotating minigun that slowly takes time to cool back down post fire!
    static double current_temp_max = 0.0;
    static double current_temp_min = 0.0;

    const double heat_rate = 0.9;  // 1.0 = instant for spikes
    const double cool_rate = 0.05; // 0.5 = (5%/frame) when quiet

    double *local_min, *local_max;

    // If this is true, we let the caller have a fixed min and max and do not
    // do any sort of calculation or storage of past values.
    if(dmax >= 0.0 && dmin >= 0.0) {
        local_max = &dmax;
        local_min = &dmin;
    } else {
        minmaxf(data, len, &local_min, &local_max);

        if(*local_max > current_temp_max) {
            // HEAT UP: We need to grow to fit this new spike
            current_temp_max = lerp(current_temp_max, *local_max, heat_rate);
        } else {
            // COOL DOWN: The spike is gone, slowly drift back down
            current_temp_max = lerp(current_temp_max, *local_max, cool_rate);
        }

        if(*local_min < current_temp_min) {
            current_temp_min = lerp(current_temp_min, *local_min, heat_rate);
        } else {
            current_temp_min = lerp(current_temp_min, *local_min, cool_rate);
        }
    }

    local_min = &current_temp_min;
    local_max = &current_temp_max;

    // Below is the old approach that just keeps a rolling memory of past
    // values and uses those to calculate min and max, before we had a
    // Double_stack type. We'll keep it for reference, as we make the new
    // one above it, with Dobule_stack.

    // static double* memory  = NULL;
    // static size_t  memsize = 0;
    // static size_t  memlen  = 0;
    // static size_t  memidx  = 0;
    //
    //
    // if(dmax < 0.0 || dmin < 0.0) {
    //     minmaxf(data, len, &local_min, &local_max);
    //
    //     if(memory == NULL) {
    //         size_t ml = (len) + len / 4;
    //         memsize   = (sizeof(double) * (ml)) + (sizeof(double) * 2);
    //         memlen    = (ml);
    //         memidx    = 0;
    //         memory    = malloc(memsize);
    //         memset(memory, 0, memsize);
    //         // memcpy(memory, data, sizeof(double) * len);
    //     }
    //
    //     memory[memidx]     = 1.0;
    //     memory[memidx + 1] = *local_max;
    //     memidx += 2;
    //
    //     if(memidx >= memlen) {
    //         memidx = 0;
    //     }
    //
    //     // double* nullme;
    //
    //     static double mins[20];
    //     static double maxs[20];
    //     static double avg_mins[20];
    //     static double avg_maxs[20];
    //     static size_t avg_idx = 0;
    //     static size_t mm_idx  = 0;
    //
    //     if(avg_idx == 0 && mm_idx == 0) {
    //         for(size_t i = 0; i < 20; ++i) {
    //             mins[i]     = *local_min;
    //             maxs[i]     = *local_max;
    //             avg_mins[i] = *local_min;
    //             avg_maxs[i] = *local_max;
    //         }
    //     }
    //
    //     if(avg_idx >= 20) {
    //         avg_idx = 19;
    //
    //         memmove(avg_mins, &avg_mins[1], 19 * sizeof(double));
    //         memmove(avg_maxs, &avg_maxs[1], 19 * sizeof(double));
    //     }
    //
    //     if(mm_idx >= 20) {
    //         mm_idx = 19;
    //
    //         memmove(mins, &mins[1], 19 * sizeof(double));
    //         memmove(maxs, &maxs[1], 19 * sizeof(double));
    //     }
    //
    //     if(mm_idx < 20) {
    //         mins[mm_idx] = *local_min;
    //         maxs[mm_idx] = *local_max;
    //         mm_idx++;
    //     }
    //
    //     // Recalculate min and max as averages
    //     double sum_min = 0.0;
    //     double sum_max = 0.0;
    //
    //     for(size_t i = 0; i < mm_idx; ++i) {
    //         sum_min += mins[i];
    //         sum_max += maxs[i];
    //     }
    //
    //     double avg_min = sum_min / (double)(mm_idx);
    //     double avg_max = sum_max / (double)(mm_idx);
    //
    //     if(avg_idx < 20) {
    //         avg_mins[avg_idx] = avg_min;
    //         avg_maxs[avg_idx] = avg_max;
    //         avg_idx++;
    //     }
    //
    //     static double combined[40];
    //     memcpy(combined, avg_mins, sizeof(double) * avg_idx);
    //     memcpy(&combined[avg_idx], avg_maxs, sizeof(double) * avg_idx);
    //
    //     // double both[40+len];
    //
    //     // memcpy(both, memory, memsize);
    //     // memcpy(both, combined, 40*sizeof(double));
    //     // memcpy(both+40, data, len*sizeof(double));
    //
    //     // minmaxf(both, 40+len, &min, &max);
    //     minmaxf(combined, 20, &local_min, &local_max);
    //
    //     if(*local_max < 10.0) {
    //         *local_max = 10.0;
    //     }
    //
    //     if(*local_min < 1.0) {
    //         *local_min = 1.0;
    //     }
    // } else {
    //     local_max = &dmax;
    //     local_min = &dmin;
    // }

    // -----------------------------------------------------------------------

    wchar_t chart[outlen];
    chart[outlen - 1] = L'\0';

    size_t idx = 0;
    size_t i   = 0;

    do {
        if(i >= len) {
            break;
        }

        double v1 = data[i];
        double v2 = data[i + 1 < len ? i + 1 : i];

        double norm1 = (v1 - *local_min) / (*local_max - *local_min);
        double perc1 = norm1 * 100;

        double norm2 = (v2 - *local_min) / (*local_max - *local_min);
        double perc2 = norm2 * 100;

        uint8_t vcode   = classify(perc1, perc2);
        wchar_t braille = BRAILLE_TABLE[vcode];

        chart[idx] = braille;
        idx++;

        i += 2;
    } while(idx < (outlen - 1));

    memcpy(out, &chart, outlen * sizeof(wchar_t));
    return idx;
}

void test_braille_chart(void)
{
    size_t ds     = 10;
    double data[] = { 13.0,   15.124,  50.1234, 19.123, 6.124,
                      62.234, 10000.0, 5000.0,  300.0,  700.0 };

    wchar_t chart[ds];
    size_t  written = write_braille_chart(chart, ds, data, ds, 100, 0);

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
