#include "color/color.h"
#include "i3bar.h"
#include "pango.h"

#include <locale.h>
#include <nvml.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>

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

    gpu_metrics_t ret = { /* return value */
                          .temperature = 0,
                          .memoryusage = 0,
                          .utilization = 0
    };

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

static inline void output(void)
{
    setlocale(LC_ALL, "");

    GradientStep* gradient = Gradient(
      Threshold(45.0, GREEN), Threshold(60.0, ORANGE), Threshold(100.0, RED));

    i3bar_block_t block;
    i3bar_block_init(&block);
    block.full_text = malloc(sizeof(wchar_t) * 64);
    block.separator = false;

    while(1) {
        gpu_metrics_t metrics = sample_gpu();

        unsigned int utilization = metrics.utilization,
                     temperature = metrics.temperature;

        block.color = map_to_color_hex(metrics.temperature, gradient);

        swprintf(
          block.full_text, 64, L"%2.u%% %u°C  ", utilization, temperature);

        i3bar_block_output(&block);
        usleep(USLEEPFOR);
    }

    free(block.full_text);
}

int main(void)
{
    output();
    return EXIT_SUCCESS;
}
