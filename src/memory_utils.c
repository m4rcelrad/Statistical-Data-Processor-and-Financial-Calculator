#include "memory_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

void* aligned_calloc(const size_t num, const size_t size, size_t alignment) {
    if (num == 0 || size == 0) return NULL;

    if (alignment < sizeof(void*) || (alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    if (size > SIZE_MAX / num) {
        return NULL;
    }

    size_t total_size = num * size;
    const size_t remainder = total_size % alignment;
    if (remainder != 0) {
        const size_t padding = alignment - remainder;

        if (total_size > SIZE_MAX - padding) {
            return NULL;
        }

        total_size += padding;
    }

    void *ptr = NULL;

#if defined(_MSC_VER)
    ptr = _aligned_malloc(total_size, alignment);
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    ptr = aligned_alloc(alignment, total_size);
#else
    if (posix_memalign(&ptr, alignment, total_size) != 0) {
        ptr = NULL;
    }
#endif

    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

void aligned_free(void *ptr) {
    if (!ptr) return;

#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}