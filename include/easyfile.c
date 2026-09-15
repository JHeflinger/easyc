#include "easyfile.h"
#include "easymemory.h"
#include "easylogger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef __WIN32
    #include <sys/types.h>
    #include <unistd.h>
    #include <dirent.h>
#else
    #define WIN32_LEAN_AND_MEAN
    #define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
    #define NOVIRTUALKEYCODES // VK_*
    #define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
    #define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
    #define NOSYSMETRICS      // SM_*
    #define NOMENUS           // MF_*
    #define NOICONS           // IDI_*
    #define NOKEYSTATES       // MK_*
    #define NOSYSCOMMANDS     // SC_*
    #define NORASTEROPS       // Binary and Tertiary raster ops
    #define NOSHOWWINDOW      // SW_*
    #define OEMRESOURCE       // OEM Resource values
    #define NOATOM            // Atom Manager routines
    #define NOCLIPBOARD       // Clipboard routines
    #define NOCOLOR           // Screen colors
    #define NOCTLMGR          // Control and Dialog routines
    #define NODRAWTEXT        // DrawText() and DT_*
    #define NOGDI             // All GDI defines and routines
    #define NOKERNEL          // All KERNEL defines and routines
    #define NOUSER            // All USER defines and routines
    #define NOMB              // MB_* and MessageBox()
    #define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
    #define NOMETAFILE        // typedef METAFILEPICT
    #define NOMSG             // typedef MSG and associated routines
    #define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
    #define NOSCROLL          // SB_* and scrolling routines
    #define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
    #define NOSOUND           // Sound driver routines
    #define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
    #define NOWH              // SetWindowsHook and WH_*
    #define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
    #define NOCOMM            // COMM driver routines
    #define NOKANJI           // Kanji support stuff.
    #define NOHELP            // Help engine interface.
    #define NOPROFILER        // Profiler interface.
    #define NODEFERWINDOWPOS  // DeferWindowPos routines
    #define NOMCX             // Modem Configuration Extensions
    #include <windows.h>
    #include <direct.h>
#endif

static const char* FileExtension(const char* path) {
    const char* dot = strrchr(path, '.');
    const char* slash1 = strrchr(path, '/');
    const char* slash2 = strrchr(path, '\\');
    const char* slash = slash1 > slash2 ? slash1 : slash2;
    if (!dot || (slash && dot < slash)) return NULL;
    return dot + 1;
}

ez_FileType ez_get_filetype(const char* path) {
    const char* extension = FileExtension(path);
    if (!extension) return UNKNOWN;
    if (strcmp(extension, "obj") == 0 || strcmp(extension, "OBJ") == 0) {
        return DOTOBJ;
    } else if (strcmp(extension, "prism") == 0 || strcmp(extension, "PRISM") == 0) {
        return DOTPRISM;
    } else if (strcmp(extension, "spv") == 0 || strcmp(extension, "SPV") == 0) {
        return DOTSPV;
    } else if (strcmp(extension, "mtl") == 0 || strcmp(extension, "MTL") == 0) {
        return DOTMTL;
    } else if (strcmp(extension, "xml") == 0 || strcmp(extension, "XML") == 0) {
        return DOTXML;
    } else if (strcmp(extension, "fbx") == 0 || strcmp(extension, "FBX") == 0) {
        return DOTFBX;
    } else if (strcmp(extension, "ssg") == 0 || strcmp(extension, "ssg") == 0) {
        return DOTSSG;
    }
    return UNKNOWN;
}

const char* ez_strip_filename(const char* path) {
    for (int i = (int)strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            if (i == (int)strlen(path) - 1) return NULL;
            return path + i + 1;
        }
    }
    return NULL;
}

ez_File* ez_load_file(const char* filename) {
	ez_File* sfile = EZ_ALLOC(1, sizeof(ez_File));
    sfile->type = ez_get_filetype(filename);
	FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        EZ_ERROR("Unable to open file \"%s\"", filename);
        EZ_FREE(sfile);
        return NULL;
    }
	fseek(file, 0, SEEK_END);
	sfile->size = ftell(file);
	rewind(file);
	sfile->data = EZ_ALLOC(sfile->size, sizeof(char));
	size_t read = fread(sfile->data, 1, sfile->size, file);
    fclose(file);
    if (read != sfile->size) {
        EZ_ERROR("Unable to read file \"%s\"", filename);
        ez_free_file(sfile);
        return NULL;
    }
	return sfile;
}

void ez_free_file(ez_File* file) {
	EZ_FREE(file->data);
	EZ_FREE(file);
}

ez_FileParser ez_parser(ez_File* file) {
    return (ez_FileParser){ file, 0, 0 };
}

BOOL ez_next_line(ez_FileParser* lp, char* buffer, size_t size) {
    if (lp->cursor >= lp->file->size) return FALSE;
    memset(buffer, 0, size);
    int last_ind = -1;
    for (size_t i = lp->cursor; i < lp->file->size; i++) {
        if (lp->file->data[i] == '\n') {
            last_ind = (int)i;
            break;
        }
    }
    lp->line++;
    if (last_ind < 0) {
        memcpy(buffer, lp->file->data + lp->cursor, lp->file->size - lp->cursor);
        lp->cursor = lp->file->size;
        return TRUE;
    }
    if (last_ind - lp->cursor > size) {
        EZ_WARN("line overflow detected when parsing for lines...");
        memcpy(buffer, lp->file->data + lp->cursor, size);
        lp->cursor += size;
        return TRUE;
    }
    memcpy(buffer, lp->file->data + lp->cursor, last_ind - lp->cursor);
    lp->cursor = last_ind + 1;
    return TRUE;
}

BOOL ez_file_exists(const char* filename) {
	#ifndef __WIN32
		struct stat statbuf;
        if (stat(filename, &statbuf) != 0) {
            return 0;
        }
        return !S_ISDIR(statbuf.st_mode);
	#else
		struct _stat statbuf;
        if (_stat(filename, &statbuf) != 0) {
            return 0;
        }
        return (statbuf.st_mode & _S_IFDIR) == 0;
	#endif
}
