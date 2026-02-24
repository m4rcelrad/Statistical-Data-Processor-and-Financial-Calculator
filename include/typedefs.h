#ifndef STATISTICALDATAPROCESSOR_TYPEDEFS_H
#define STATISTICALDATAPROCESSOR_TYPEDEFS_H

/**
 * @file typedefs.h
 * @brief Global type definitions and platform-specific shims.
 */

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#endif // STATISTICALDATAPROCESSOR_TYPEDEFS_H
