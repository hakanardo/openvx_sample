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

#ifndef _OPENVX_INT_ARRAY_H_
#define _OPENVX_INT_ARRAY_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

void vxPrintArray(vx_array array);

vx_array vxCreateArrayInt(vx_context context, vx_enum item_type, vx_size capacity, vx_bool is_virtual, vx_enum type);

void vxDestructArray(vx_reference reference);

void vxReleaseArrayInt(vx_array *array);

vx_bool vxInitVirtualArray(vx_array array, vx_enum item_type, vx_size capacity);

vx_bool vxValidateArray(vx_array array, vx_enum item_type, vx_size capacity);

vx_bool vxAllocateArray(vx_array array);

vx_status vxAccessArrayRangeInt(vx_array array, vx_size start, vx_size end, void **ptr, vx_enum usage);
vx_status vxCommitArrayRangeInt(vx_array array, vx_size start, vx_size end, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
