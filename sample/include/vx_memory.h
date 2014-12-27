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

#ifndef _OPENVX_INT_MEMORY_H_
#define _OPENVX_INT_MEMORY_H_

/*! \file
 * \brief Defines an API for memory operations.
 *
 * \defgroup group_int_memory Internal Memory API.
 * \ingroup group_internal
 * \brief The Internal Memory API.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Frees a memory block.
 * \ingroup group_int_memory
 */
vx_bool vxFreeMemory(vx_context_t *context, vx_memory_t *memory);

/*! \brief Allocates a memory block.
 * \ingroup group_int_memory
 */
vx_bool vxAllocateMemory(vx_context_t *context, vx_memory_t *memory);

void vxPrintMemory(vx_memory_t *mem);

vx_size vxComputeMemorySize(vx_memory_t *memory, vx_uint32 p);

#ifdef __cplusplus
}
#endif

#endif

