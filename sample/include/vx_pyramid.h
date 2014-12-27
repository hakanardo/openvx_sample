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

#ifndef _OPENVX_INT_PYRAMID_H_
#define _OPENVX_INT_PYRAMID_H_

/*!
 * \file
 * \brief The internal pyramid implementation
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_pyramid Internal Pyramid API
 * \ingroup group_internal
 * \brief The Internal Pyramid API.
 */

#ifdef __cplusplus
    extern "C" {
#endif

/*! \brief Releases a pyramid with internal references.
 * \param [in] pyramid The pyramid to release.
 * \ingroup group_int_pyramid
 */
void vxReleasePyramidInt(vx_pyramid pyramid);

/*! \brief Initializes the internals of a pyramid structure
 * \ingroup group_int_pyramid
 */
vx_status vxInitPyramid(vx_pyramid pyramid,
                        vx_size levels,
                        vx_float32 scale,
                        vx_uint32 width,
                        vx_uint32 height,
                        vx_df_image format);

/*! \brief Destroys a pyrmid object.
 * \ingroup group_int_pyramid
 */
void vxDestructPyramid(vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif
