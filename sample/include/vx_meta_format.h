/*
 * Copyright (c) 2014-2014 The Khronos Group Inc.
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

#ifndef _OPENVX_INT_META_FORMAT_H_
#define _OPENVX_INT_META_FORMAT_H_

/*!
 * \file
 * \brief The internal meta format implementation
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_meta_format The Meta-Format API
 * \ingroup group_internal
 * \brief The Internal Meta Format API
 */

#ifdef __cplusplus
    extern "C" {
#endif

/*! \brief Releases a meta-format object.
 * \param [in,out] pmeta
 * \ingroup group_int_meta_format
 */
void vxReleaseMetaFormat(vx_meta_format *pmeta);

/*! \brief Creates a metaformat object.
 * \param [in] context The overall context object.
 * \ingroup group_int_meta_format
 */
vx_meta_format vxCreateMetaFormat(vx_context context);

#ifdef __cplusplus
}
#endif

#endif
