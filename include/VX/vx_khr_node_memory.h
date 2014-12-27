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

#ifndef _VX_KHR_NODE_MEMORY_H_
#define _VX_KHR_NODE_MEMORY_H_

/*! \brief The Node Memory Extension.
 * \file
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#define OPENVX_KHR_NODE_MEMORY      "vx_khr_node_memory"

/*! \brief The kernel object attributes for global and local memory.
 * \ingroup group_kernel
 */
enum vx_kernel_attribute_memory_e {
    /*! \brief The global data pointer size to be shared across all instances of
     * the kernel (nodes are instances of kernels).
     * Use a \ref vx_size parameter.
     * \note If not set it will default to zero.
     */
    VX_KERNEL_ATTRIBUTE_GLOBAL_DATA_SIZE = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_KERNEL) + 0x5,
    /*! \brief The global data pointer to the shared across all the instances of
     * the kernel (nodes are instances of the kernels).
     * Use a \ref void * parameter.
     */
    VX_KERNEL_ATTRIBUTE_GLOBAL_DATA_PTR = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_KERNEL) + 0x6,
};

/*! \brief The node object attributes for global and local memory.
 * \ingroup group_node
 */
enum vx_node_attribute_memory_e {
    /*! \brief Used to indicate the size of the shared kernel global memory area.
     * Use a \ref vx_size parameter.
     */
    VX_NODE_ATTRIBUTE_GLOBAL_DATA_SIZE = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_NODE) + 0x5,
    /*! \brief Used to indicate the pointer to the shared kernel global memory area.
     * Use a void * parameter.
     */
    VX_NODE_ATTRIBUTE_GLOBAL_DATA_PTR = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_NODE) + 0x6,
};

#endif

