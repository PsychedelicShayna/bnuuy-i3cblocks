#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

#include "color/color.h"
#include "i3bar.h"
#include "pango2.h"

#include "meteo.h"
#include "private.h"

#include "common.h"

#ifndef USLEEPFOR
#define USLEEPFOR 1000000
#endif /* ifndef USLEEPFOR */

#include <time.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

#define printf(format, ...)        wprintf(L##format, ##__VA_ARGS__)
#define fprintf(file, format, ...) fwprintf(file, L##format, ##__VA_ARGS__)

#define DEBUG

typedef struct {
    char*  data;
    size_t size;
} String;

void String_free(String* str)
{
    if(str->data) {
        free(str->data);
        str->data = NULL;
    }

    str->size = 0;
}

String String_clone(const String* other)
{
    String newstr = { .data = NULL, .size = 0 };

    if(other->data) {
        newstr.size = other->size;
        newstr.data = calloc(newstr.size + 1, sizeof(char));
        memcpy(newstr.data, other->data, newstr.size);
        newstr.data[newstr.size] = '\0';
    }

    return newstr;
}

// Strips trailing and leading whitespace from String s in place.
void String_strip(String* s)
{

    // Strip leading whitespace.
    size_t start = 0;

    while(start < s->size &&
          (s->data[start] == ' ' || s->data[start] == '\n' ||
           s->data[start] == '\t' || s->data[start] == '\r')) {
        ++start;
    }

    // Strip trailing whitespace.
    size_t end = s->size;

    while(end > start &&
          (s->data[end - 1] == ' ' || s->data[end - 1] == '\n' ||
           s->data[end - 1] == '\t' || s->data[end - 1] == '\r')) {
        --end;
    }

    // Now start is the index of the first non-whitespace character,
    // and end is one past the last non-whitespace character.
    size_t new_size = end - start;

    if(start > 0 && new_size > 0) {
        memmove(s->data, s->data + start, new_size);
    }

    s->data[new_size] = '\0';
    s->size           = new_size;
}

size_t _query_weather_curl_callback(void*  contents,
                                    size_t size,
                                    size_t nmemb,
                                    void*  userp)
{
    const static size_t max_size  = 1024 * 16;
    size_t              real_size = size * nmemb;

    // printf("Real size: %zu\n", real_size);

    // 16 KiB maximum. Typically around ~8.805 KiB wwith largest format.
    if(real_size > max_size) {
        fprintf(stderr,
                "size %zu > %zu failed in _query_weather_curl_callback@:%d",
                real_size,
                max_size,
                __LINE__);

        return 0;
    }

    String* mem = (String*)userp;

    // Expand memory buffer to ensure it can hold the new data from contents.
    char* ptr =
      realloc(mem->data, align(mem->size + (real_size + 1), alignof(void*)));

    if(!ptr) {
        fprintf(stderr,
                "realloc() failed in _query_weather_curl_callback@%s:%d",
                __FILE__,
                __LINE__);

        return 0;
    }

    // Realloc was ok, assign the new pointer.
    mem->data = ptr;

    // Write the contents into the buffer.
    memcpy(&(mem->data[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->data[mem->size] = '\0';

    // printf("Returned: %s\n", mem->data[mem->size]);
    wprintf(L"Contents: %s\n", (char*)contents);

    return real_size;
}

CURLcode query_weather(const char* city, String* response)
{
    char url[256];
    snprintf(url, sizeof(url), "https://wttr.in/%s?format=1", city);

    CURL*    curl;
    CURLcode res = 1;

    curl = curl_easy_init();

    if(!curl) {
        fwprintf(
          stderr, L"curl_easy_init() failed in %s:%d\n", __FILE__, __LINE__);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Give it a generous 30 seconds to respond; wttr.in can be slow.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

#ifdef DEBUG
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    *response = (String) { .size = 0, .data = NULL };

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _query_weather_curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    CURLcode result = curl_easy_perform(curl);

    if(result != CURLE_OK) {
        fwprintf(
          stderr, L"curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        if(response->data) {
            free(response->data);
            response->data = NULL;
            response->size = 0;
        }

        return result;
    }

    curl_easy_cleanup(curl);

    return 0;
}

static const char* clocks[24]  = { "󱑖", "󱑋", "󱑌", "󱑍", "󱑎",
                                   "󱑏", "󱑐", "󱑑", "󱑒", "󱑓",
                                   "󱑔", "󱑕", "󱑊", "󱐿", "󱑀",
                                   "󱑁", "󱑂", "󱑃", "󱑄", "󱑅",
                                   "󱑆", "󱑇", "󱑈", "󱑉" };
static const char* seasons[12] = {
    "", "", "󰲓", "󰲓", "󰲓", "",
    "", "", "󰉊", "󰉊", "󰉊", ""
};

static const Color season_colors[12] = { YELLOW, YELLOW, ORANGE, ORANGE,
                                         ORANGE, BLUE,   BLUE,   BLUE,
                                         GREEN,  GREEN,  GREEN,  YELLOW };

void output(void)
{

    setlocale(LC_ALL, "");

    i3bar_block_t i3b;
    i3bar_block_init(&i3b);

    i3b.full_text = calloc(PANGO_SZMAX * 8, sizeof(wchar_t));
    i3b.markup    = "pango";

    panspan psa;
    panspan_init(&psa);
    psa.foreground = rgbx(GREEN);

    time_t     t  = time(NULL);
    struct tm* lt = NULL;

    double temperature = -1;

    temperature = get_current_temperature_2m(LOCATION1_LAT, LOCATION1_LON);

    for(uint64_t i = 0; i < UINT64_MAX; ++i) {
        usleep(USLEEPFOR);

        setlocale(LC_ALL, "");
        fflush(stdout);

        t  = time(NULL);
        lt = localtime(&t);

        // Update weater every 2 minutes if usleep is 1 second.
        if(i == (60 * 2)) {
            i = 1;

            temperature =
              get_current_temperature_2m(LOCATION1_LAT, LOCATION1_LON);
        }

        char tf[24] = { 0 };
        strftime(tf, sizeof(tf) / sizeof(tf[0]), "%a%d%b%H%M%S", lt);

        // clang-format off
        i3bcat(
          &i3b,

          // Weather
          wpomf(modspan(psa, .foreground = rgbx(YELLOW)),
                PAT_ITAL | PAT_SMAL, "  %.02lf°C ", temperature ),

          wpomf(modspan(psa, .foreground = rgbx(FG_DIM)),
                PAT_ITAL,
                // F r i  0 9  J a n 
                " %c%c%c %c%c %c%c%c",
                // J       a       n
                tf[5],  tf[6],  tf[7],  
                // 0       9
                tf[3],  tf[4],  
                // F       r       i
                tf[0],  tf[1],  tf[2]), 

          wpomf(modspan(psa, .foreground = rgbx(season_colors[lt->tm_mon])),
                PAT_NULL,
                " %s  ",
                seasons[lt->tm_mon]),

          wpomf(modspan(psa, .foreground = rgbx(FG_DIM)),
                PAT_ITAL,
               // 2 3: 5 1: 4 2
                "%c%c:%c%c:%c%c ",
                // H       H
                tf[8],  tf[9],
                // M       M
                tf[10], tf[11],
                // S       S
                tf[12], tf[13]),

          wpomf(modspan(psa, .foreground = rgbx(AQUA)),
                PAT_NULL,
                "%s  ",
                clocks[lt->tm_hour]));
        // clang-format on

        i3bar_block_output(&i3b);
        memset(i3b.full_text, 0, PANGO_SZMAX * 8);
    }

    free(i3b.full_text);
}

int main(void)
{
    output();
}
