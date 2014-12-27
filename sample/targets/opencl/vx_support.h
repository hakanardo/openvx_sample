/*
 * Copyright (c) 2011-2014 The Khronos Group Inc.
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

#include <vx_interface.h>
#include <vx_internal.h>

#if defined(__ANDROID__) || defined(__linux__) || defined(__QNX__) || defined(__CYGWIN__) || defined(__APPLE__)
#if !defined(__QNX__) && !defined(__APPLE__)
#include <features.h>
#else
#define __EXT_UNIX_MISC //Needed by QNX version of dirent.h to include scandir()
#endif
#include <sys/types.h>
#if defined(__APPLE__)
#include <sys/dirent.h>
#endif
#include <dirent.h>
#include <fnmatch.h>
#define EXPERIMENTAL_USE_FNMATCH
#elif defined(_WIN32)
#define snprintf _snprintf
#endif

#define CL_MAX_LINESIZE (1024)

#define ALLOC(type,count)                               (type *)calloc(count, sizeof(type))
#define CL_ERROR_MSG(err, string)                       clPrintError(err, string, __FUNCTION__, __FILE__, __LINE__)
#define CL_BUILD_MSG(err, string)                       clBuildError(err, string, __FUNCTION__, __FILE__, __LINE__)

char *clLoadSources(char *filename, size_t *programSize);
cl_int clBuildError(cl_int build_status, const char *label, const char *function, const char *file, int line);
cl_int clPrintError(cl_int err, const char *label, const char *function, const char *file, int line);
