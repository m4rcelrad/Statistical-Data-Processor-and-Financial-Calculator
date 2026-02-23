#ifndef STATISTICALDATAPROCESSOR_MEMORY_UTILS_H
#define STATISTICALDATAPROCESSOR_MEMORY_UTILS_H

#include <stddef.h>

#define CACHE_LINE_SIZE 64

void* aligned_calloc(size_t num, size_t size, size_t alignment);

void aligned_free(void *ptr);

#endif