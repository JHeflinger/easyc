#include "easyprofile.h"
#include "easylogger.h"
#include "easytime.h"
#include <string.h>

float ez_profile_result(ez_Profiler* profiler) {
    return profiler->average;
}

void ez_configure_profile(ez_Profiler* profiler, const char* name, size_t step) {
    if (step >= EZ_PROFILER_MAX_DATASTREAM) EZ_FATAL("Profiler step size too big");
    profiler->name = name;
    profiler->step = step;
}

void ez_begin_profile(ez_Profiler* profiler) {
    profiler->curr = ez_get_time();
}

void ez_end_profile(ez_Profiler* profiler) {
    profiler->curr = ez_get_time() - profiler->curr;
    profiler->curr = profiler->curr * 1000.0;
    uint64_t copy[EZ_PROFILER_MAX_DATASTREAM];
    memcpy(copy, profiler->datastream, EZ_PROFILER_MAX_DATASTREAM * sizeof(double));
    memcpy(profiler->datastream + 1, copy, (EZ_PROFILER_MAX_DATASTREAM - 1) * sizeof(double));
    profiler->datastream[0] = profiler->curr;
    profiler->average = 0.0f;
    for (size_t i = 0; i < profiler->step; i++)
        profiler->average += (float)profiler->datastream[i] / (float)profiler->step;
}
