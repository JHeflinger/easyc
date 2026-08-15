#ifndef EASYPARSE_H
#define EASYPARSE_H

#include "easybool.h"
#include "easymemory.h"
#include <stddef.h>

BOOL ez_parse_float(const char* str, float* value);

BOOL ez_parse_size(const char* str, size_t* value);

BOOL ez_parse_bool(const char* str, BOOL* value);

char* ez_reconstruct_command(char** argv, int argc);

#endif
