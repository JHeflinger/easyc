#ifndef EASYPROFILE_H
#define EASYPROFILE_H

#include <stdint.h>
#include <stddef.h>

#define EZ_PROFILER_MAX_DATASTREAM 128

typedef struct {
    double datastream[EZ_PROFILER_MAX_DATASTREAM];
    float average;
    const char* name;
    size_t step;
    double curr;
} ez_Profiler;

float ez_profile_result(ez_Profiler* profiler);

void ez_configure_profile(ez_Profiler* profiler, const char* name, size_t step);

void ez_begin_profile(ez_Profiler* profiler);

void ez_end_profile(ez_Profiler* profiler);

#endif
