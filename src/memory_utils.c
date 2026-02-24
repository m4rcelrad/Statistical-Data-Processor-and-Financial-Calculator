#include "memory_utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

void *aligned_calloc(const size_t num, const size_t size, size_t alignment)
{
    /* Validate input sizes to prevent zero-byte allocations */
    if (num == 0 || size == 0)
        return NULL;

    /* Ensure alignment is at least the size of a pointer and is a power of 2 */
    if (alignment < sizeof(void *) || (alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    /* Prevent overflow during total size calculation */
    if (size > SIZE_MAX / num) {
        return NULL;
    }

    size_t total_size = num * size;
    const size_t remainder = total_size % alignment;

    /* Pad total_size so it is a multiple of the alignment */
    if (remainder != 0) {
        const size_t padding = alignment - remainder;

        /* Prevent overflow when adding padding */
        if (total_size > SIZE_MAX - padding) {
            return NULL;
        }

        total_size += padding;
    }

    void *ptr = NULL;

    /* Platform-specific aligned allocation */
#if defined(_MSC_VER)
    ptr = _aligned_malloc(total_size, alignment);
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    /* C11 aligned allocation */
    ptr = aligned_alloc(alignment, total_size);
#else
    /* POSIX fallback */
    if (posix_memalign(&ptr, alignment, total_size) != 0) {
        ptr = NULL;
    }
#endif

    /* Zero-initialize the memory block, matching the behavior of calloc */
    if (ptr) {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

void aligned_free(void *ptr)
{
    if (!ptr)
        return;

    /* Platform-specific aligned deallocation */
#if defined(_MSC_VER)
    _aligned_free(ptr);
#else
    /* For C11 and POSIX, standard free() successfully handles aligned memory */
    free(ptr);
#endif
}
