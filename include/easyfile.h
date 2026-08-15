#ifndef EASYFILE_H
#define EASYFILE_H

#include <stddef.h>
#include "easybool.h"

typedef enum {
    UNKNOWN = 0,
    DOTPRISM,
    DOTOBJ,
    DOTSPV,
    DOTMTL,
    DOTXML,
    DOTFBX
} ez_FileType;

typedef struct {
	char* data;
	size_t size;
    ez_FileType type;
} ez_File;

typedef struct {
    ez_File* file;
    size_t line;
    size_t cursor;
} ez_FileParser;

ez_FileType ez_get_filetype(const char* path);

const char* ez_strip_filename(const char* path);

ez_File* ez_load_file(const char* filename);

void ez_free_file(ez_File* file);

ez_FileParser ez_parser(ez_File* file);

BOOL ez_next_line(ez_FileParser* lp, char* buffer, size_t size);

#endif
