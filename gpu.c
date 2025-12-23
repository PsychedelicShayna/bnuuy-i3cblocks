#include "i3blocks_common.h"

#include <nvml.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef USLEEPFOR
    #define USLEEPFOR 1000000
#endif

// to avoid error handling boilerplate repetition (:
#define safeNvmlCall(err, call)             \
    err = call;                             \
    if(err != NVML_SUCCESS) {               \
        fprintf(stderr,                     \
                "%s:%d NVML Failure: %s\n", \
                __FILE__,                   \
                __LINE__,                   \
                nvmlErrorString(err));      \
    }

typedef struct {
    unsigned temperature;
    unsigned utilization;
    unsigned memoryusage;
} gpu_metrics_t;

gpu_metrics_t sample_gpu(void)
{
    nvmlDevice_t      dev; // handle to GPU
    nvmlReturn_t      err; // error code if any
    nvmlUtilization_t use; // % utilization for vram and gpu

    gpu_metrics_t ret = {/* return value */
                         .temperature = 0,
                         .memoryusage = 0,
                         .utilization = 0};

    safeNvmlCall(err, nvmlInit_v2());
    safeNvmlCall(err, nvmlDeviceGetHandleByIndex(0, &dev));

    safeNvmlCall(
        err,
        nvmlDeviceGetTemperature(dev, NVML_TEMPERATURE_GPU, &ret.temperature));

    safeNvmlCall(err, nvmlDeviceGetUtilizationRates(dev, &use));

    ret.utilization = use.gpu;
    ret.memoryusage = use.memory;

    return ret;
}

static inline void output(GB_COLOR color, gpu_metrics_t metrics)
{
    char full_text[64], out[256];

    sprintf(full_text, "%2.u%% %u°C", metrics.utilization, metrics.temperature);

    sprintf(out, i3bjt, full_text, color);
    fprintf(stdout, "%s", out);
    fflush(stdout);
}

// ${UTIL}% ${TEMP}°C "
void output_loop(atomic_bool* alive)
{
    GB_Color_Transition_Step baseline_green = {
        .threshold  = 45,
        .transition = GB_RGB_GREEN,
    };
    GB_Color_Transition_Step green_to_orange = {
        .threshold  = 60,
        .transition = GB_RGB_ORANGE,
    };
    GB_Color_Transition_Step orange_to_red = {
        .threshold  = 100.0,
        .transition = GB_RGB_RED,
    };

    GB_Color_Transition_Step steps[3];

    steps[0] = baseline_green;
    steps[1] = green_to_orange;
    steps[2] = orange_to_red;

    while(*alive) {
        gpu_metrics_t metrics = sample_gpu();

        GB_COLOR color =
            gb_map_percent((double)metrics.temperature, steps, 3, GB_GREEN);

        output(color, metrics);
        usleep(USLEEPFOR);
    }
}

int main(void)
{

    atomic_bool alive;
    atomic_store(&alive, 1);
    output_loop(&alive);
    return EXIT_SUCCESS;
}
