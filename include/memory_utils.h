#ifndef STATISTICALDATAPROCESSOR_MEMORY_UTILS_H
#define STATISTICALDATAPROCESSOR_MEMORY_UTILS_H

#include <stddef.h>

/**
 * @file memory_utils.h
 * @brief Memory allocation utilities with cache-line alignment support.
 *
 * This module provides functions for allocating memory that is properly aligned
 * to specific memory boundaries (such as a CPU cache line). This helps prevent
 * false sharing and improves overall memory access performance.
 */

/**
 * @brief Typical size of a CPU cache line in bytes.
 * Aligning data structures to this boundary can significantly improve performance
 * in data-intensive and concurrent computing operations.
 */
#define CACHE_LINE_SIZE 64

/**
 * @brief Allocates and zero-initializes memory aligned to a specific boundary.
 *
 * This function acts similarly to the standard calloc() but ensures that the
 * starting address of the allocated block is a multiple of the given alignment.
 *
 * @param num The number of elements to allocate.
 * @param size The size in bytes of each element.
 * @param alignment The byte boundary to align the memory to (must be a power of 2).
 * @return A pointer to the aligned, zero-initialized memory block, or NULL if allocation fails.
 */
void *aligned_calloc(size_t num, size_t size, size_t alignment);

/**
 * @brief Frees memory allocated by aligned_calloc.
 *
 * It correctly delegates the deallocation to the platform-specific aligned free
 * function (e.g., _aligned_free on MSVC, or standard free elsewhere).
 *
 * @param ptr Pointer to the memory block to free. If NULL, the function does nothing.
 */
void aligned_free(void *ptr);

#endif
