#ifndef _WEATHER_H
#define _WEATHER_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include "common.h"
#include "private.h"

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

#define API_ENDPOINT "api.open-meteo.com/v1/forecast"

typedef struct {
    size_t size;
    size_t capacity;
    char*  data;
} growbuf_t;

size_t cb_curl_recv_data(void* chunk, size_t size, size_t nmemb, void* outvoid)
{
    growbuf_t* out = (growbuf_t*)outvoid;

    const static size_t max_size = 1024 * 1024; // 1 MiB
    const static size_t step     = 4096;

    debugf("size: %zu\nnmemb: %zu\n", size, nmemb);
    size_t real_size = nmemb * size;

    if(out == NULL) {
        debug("growbuf_t was NULL! provide an initialized growbuf_t to store "
              "the data");

        return 0;
    }

    // If size ever greater than capacity, something went very wrong.
    assert(out->size <= out->capacity);

    if(out->size + real_size >= out->capacity) {
        size_t new_size =
          (out->capacity + step * (size_t)ceilf((float)(real_size + 1) / step));

        if(new_size >= max_size) {
            return 0;
        }

        assert(new_size < max_size && "new realloc of would exceed maxsize");

        char* ptr = reallocarray(out->data, new_size, sizeof(char));
        assert(ptr != NULL && "reallocarray() returned NULL (OOM)");
        memset(ptr + out->size, 0, new_size - out->size);

        out->data     = ptr;
        out->capacity = new_size;

        debugf("Reallocated memory to %zu, %zu", out->size, out->capacity);
    }

    debugf("Chunk Size: %zu\nN: %zu\n", size, nmemb);

    memcpy(out->data + out->size, chunk, real_size);
    out->size += real_size;

    return real_size;
}

/* Returns 0 on success. 
 * If negative returned, there was an error in the function. 
 * If positive is returned, it is a `CURLcode` error enum value, do a cast.
 * Writes temperature to `double* out` at `latitude` & `longitude`
 * */
int get_current_temperature_2m(double* out, double latitude, double longitude)
{

    char url[256];
    memset(url, 0, 256);

    sprintf(url,
            "https://" API_ENDPOINT
            "/?latitude=%lf&longitude=%lf&current=temperature_2m",
            latitude,
            longitude);

    debugf("URL: %s\n", url);

    CURL* curl = curl_easy_init();

    growbuf_t growbuf = (growbuf_t) { .data = NULL, .capacity = 0, .size = 0 };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb_curl_recv_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &growbuf);

#ifdef DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    CURLcode err = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(err != CURLE_OK) {
        debugf("curl_easy_perform() failed: %s\n", curl_easy_strerror(err));
        return (int)err;
    }

    debugf("Growbuf: size %zu; cap %zu; data (%p)------\n%s\n-----\n",
           growbuf.size,
           growbuf.capacity,
           growbuf.data,
           growbuf.data);

    if(growbuf.size == 0) {
        return -2;
    }

    struct json_object* pj = NULL;

    pj = json_tokener_parse(growbuf.data);

    if(pj == NULL) {
        return -3;
    }

    struct json_object* current        = jgeto(pj, current);
    double              temperature_2m = jget(double, current, temperature_2m);

    json_object_put(pj);
    free(growbuf.data);

    if(out != NULL) {
        *out = temperature_2m;
    }

    return 0;
}

#ifdef DEBUG

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    double temp = get_current_temperature_2m(LOCATION1_LAT, LOCATION1_LON);
    printf("Temperature: %lf\n", temp);

    curl_global_cleanup();
    return EXIT_SUCCESS;
}

#endif

#endif // !_WEATHER_H
