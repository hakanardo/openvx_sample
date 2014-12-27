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
#ifndef _OPENVX_EXT_XYZ_H_
#define _OPENVX_EXT_XYZ_H_

/*!
 * \file
 * \brief An example of how to wrap a User Extension Kernel.
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_xyz_ext The Example User Kernel Extension
 *
 */

#include <VX/vx.h>

/*!
 * \file vx_ext_xyz.h
 * \brief The example header for how to write a user mode extension to OpenVX.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

/*! \brief The XYZ Data area in bytes
 * \ingroup group_xyz_ext
 */
#define XYZ_DATA_AREA (1024)

/*! \brief The required number of items int the temp array
 * \ingroup group_xyz_ext
 */
#define XYZ_TEMP_NUMITEMS (374)

/*! \brief The minimum value of the scalar for the XYZ Kernel.
 * \ingroup group_xyz_ext
 */
#define XYZ_VALUE_MIN   (-10)

/*! \brief The maximum value of the scalar for the XYZ Kernel.
 * \ingroup group_xyz_ext
 */
#define XYZ_VALUE_MAX  (10)

//! [KERNEL ENUM]
#define VX_KERNEL_NAME_KHR_XYZ "org.khronos.example.xyz"
/*! \brief The XYZ Example Library Set
 * \ingroup group_xyz_ext
 */
#define VX_LIBRARY_XYZ (0x3) // assigned from Khronos, vendors control their own

/*! \brief The list of XYZ Kernels.
 * \ingroup group_xyz_ext
 */
enum vx_kernel_xyz_ext_e {
    /*! \brief The Example User Defined Kernel */
    VX_KERNEL_KHR_XYZ = VX_KERNEL_BASE(VX_ID_KHRONOS, VX_LIBRARY_XYZ) + 0x0,
    // up to 0xFFF kernel enums can be created.
};
//! [KERNEL ENUM]

#ifdef __cplusplus
extern "C" {
#endif

//! [node]
/*! \brief [Graph] This is an example ISV or OEM provided node which executes
 * in the Graph to call the XYZ kernel.
 * \param [in] graph The handle to the graph in which to instantiate the node.
 * \param [in] input The input image.
 * \param [in] value The input scalar value
 * \param [out] output The output image.
 * \param [in,out] temp A temp array for some data which is needed for
 * every iteration.
 * \ingroup group_example_kernel
 */
vx_node vxXYZNode(vx_graph graph, vx_image input, vx_uint32 value, vx_image output, vx_array temp);
//! [node]

//! [vxu]
/*! \brief [Immediate] This is an example of an immediate mode version of the XYZ node.
 * \param [in] context The overall context of the implementation.
 * \param [in] input The input image.
 * \param [in] value The input scalar value
 * \param [out] output The output image.
 * \param [in,out] temp A temp array for some data which is needed for
 * every iteration.
 * \ingroup group_example_kernel
 */
vx_status vxuXYZ(vx_context context, vx_image input, vx_uint32 value, vx_image output, vx_array temp);
//! [vxu]

#ifdef __cplusplus
}
#endif

#endif
