#ifndef EASYSORT_H
#define EASYSORT_H

#include "easyobjects.h"

#define DECLARE_EASYSORT(T) \
void EasySort_##T(ARRLIST_##T* list, float (*score)(T));

#define IMPL_EASYSORT(T) \
void EasySort_##T(ARRLIST_##T* list, float (*score)(T)) { \
    if (list->size <= 1) return; \
    uint8_t* removed = (uint8_t*)EZ_ALLOC(list->size, sizeof(uint8_t)); \
    memset(removed, 0, list->size * sizeof(uint8_t)); \
    size_t removed_count = 0; \
    float prev_score = score(list->data[0]); \
    for (size_t i = 1; i < list->size; i++) { \
        float s = score(list->data[i]); \
        if (s < prev_score) { \
            removed[i] = 1; \
            removed_count++; \
        } else { \
            prev_score = s; \
        } \
    } \
    T* displaced = (T*)EZ_ALLOC(removed_count, sizeof(T)); \
    float* displaced_scores = (float*)EZ_ALLOC(removed_count, sizeof(float)); \
    size_t d = 0; \
    for (size_t i = 0; i < list->size; i++) { \
        if (removed[i]) { \
            displaced[d] = list->data[i]; \
            displaced_scores[d] = score(list->data[i]); \
            d++; \
        } \
    } \
    size_t write = 0; \
    for (size_t i = 0; i < list->size; i++) { \
        if (!removed[i]) { \
            if (write != i) \
                memcpy(&list->data[write], &list->data[i], sizeof(T)); \
            write++; \
        } \
    } \
    list->size = write; \
    EZ_FREE(removed); \
    for (size_t i = 0; i < removed_count; i++) { \
        float s = displaced_scores[i]; \
        size_t lo = 0, hi = list->size; \
        while (lo < hi) { \
            size_t mid = lo + (hi - lo) / 2; \
            if (score(list->data[mid]) <= s) lo = mid + 1; \
            else hi = mid; \
        } \
        if (list->size >= list->maxsize) { \
            list->maxsize = list->maxsize == 0 ? 1 : list->maxsize * 2; \
            list->data = EZ_REALLOC(list->data, list->maxsize, sizeof(T)); \
        } \
        memmove(&list->data[lo + 1], &list->data[lo], (list->size - lo) * sizeof(T)); \
        memcpy(&list->data[lo], &displaced[i], sizeof(T)); \
        list->size++; \
    } \
    \
    EZ_FREE(displaced); \
    EZ_FREE(displaced_scores); \
}

#endif
