/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#pragma once
#ifndef _STDINT_ALTERNATE_H_
#define _STDINT_ALTERNATE_H_

#if defined(WIN32) || defined(_WIN64)

#include <windows.h>

/* some windows incompatibility issues */
#ifndef __attribute__
#define __attribute__(x)
#endif

#endif

/*! \file
 * \brief This is a work-alike for platforms that don't have stdint like Windows.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

typedef signed char      int8_t;
typedef unsigned char   uint8_t;

typedef signed short    int16_t;
typedef unsigned short uint16_t;

typedef signed int      int32_t;
typedef unsigned int   uint32_t;

#if defined(ARCH_32)
#if defined(WIN32)
typedef __int64           int64_t;
typedef unsigned __int64 uint64_t;
#else
typedef signed long long    int64_t;
typedef unsigned long long uint64_t;
#endif
typedef int32_t intmax_t;
typedef uint32_t uintmax_t;
#elif defined(ARCH_64)
#if defined(_WIN64)
typedef __int64             int64_t;
typedef unsigned __int64   uint64_t;
#else
typedef signed long         int64_t;
typedef unsigned long      uint64_t;
#endif
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
#endif

#include <limits.h>

#define INT8_MAX    SCHAR_MAX
#define INT8_MIN    SCHAR_MIN
#define INT16_MAX   SHRT_MAX
#define INT16_MIN   SHRT_MIN
#define INT32_MAX   INT_MAX
#define INT32_MIN   INT_MIN
#define INT64_MIN   _I64_MIN
#define INT64_MAX   _I64_MAX
#define UINT8_MAX   UCHAR_MAX
#define UINT16_MAX  USHRT_MAX
#define UINT32_MAX  UINT_MAX
#define UINT64_MAX  ((INT64_MAX * 2ULL) + 1ULL)

#endif // _STDINT_ALTERNATE_H_

