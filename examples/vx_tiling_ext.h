/*
 * Copyright (c) 2013-2014 The Khronos Group Inc.
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

#ifndef _VX_EXAMPLE_TILING_H_
#define _VX_EXAMPLE_TILING_H_

/*! \brief The list of example tiling kernels */
enum vx_tiling_kernels_e {
    /*! \brief a re-implementation of a gaussian blur */
    VX_KERNEL_GAUSSIAN_3x3_TILING = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_BASE) + 0x100,
    /*! \brief a re-implementation of a generic box filter of any size */
    VX_KERNEL_BOX_MxN_TILING = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_BASE) + 0x101,
    /*! \brief a re-implementation of a add kernel */
    VX_KERNEL_ADD_TILING = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_BASE) + 0x102,
    /*! \brief an alpha multiply kernel */
    VX_KERNEL_ALPHA_TILING = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_KHR_BASE) + 0x103,
};

#ifdef _VX_TILING_EXT_INTERNAL_
void add_image_tiling(void * VX_RESTRICT parameters[VX_RESTRICT], void * VX_RESTRICT tile_memory, vx_size tile_memory_size);
void alpha_image_tiling(void * VX_RESTRICT parameters[VX_RESTRICT], void * VX_RESTRICT tile_memory, vx_size tile_memory_size);
void box_image_tiling(void * VX_RESTRICT parameters[VX_RESTRICT], void * VX_RESTRICT tile_memory, vx_size tile_memory_size);
void gaussian_image_tiling_fast(void * VX_RESTRICT parameters[VX_RESTRICT], void * VX_RESTRICT tile_memory, vx_size tile_memory_size);
#endif

#endif
