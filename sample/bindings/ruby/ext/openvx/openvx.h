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

#ifndef _OPENVX_RUBY_H_
#define _OPENVX_RUBY_H_

/*!
 * \file
 * \brief The OpenVX Ruby Bindings
 * \author Erik Rainey <erik.rainey@gmail.com>
 *  \note There are several rules about using the Extensions API.
 * 1.) VALUE can take a POINTER (it's an unsigned long).
 * 2.) Check_Type is required for every parameter
 */

#include <stdint.h>
#include <VX/vx.h>
#include <rubyext.h>

/*! \note This must match the order in \ref objects */
enum _vx_object_e {
    VX_OBJ_REF,
    VX_OBJ_KERNEL,
    VX_OBJ_GRAPH,
    VX_OBJ_IMAGE,
    VX_OBJ_NODE,
    VX_OBJ_PARAMETER,
    VX_OBJ_BUFFER,
    VX_OBJ_SCALAR,
    VX_OBJ_DISTRIBUTION,
    VX_OBJ_LUT,
    VX_OBJ_CONVOLUTION,
    VX_OBJ_MATRIX,
    VX_OBJ_THRESHOLD,
    // not really objects...
    VX_OBJ_KEYPOINT,
    VX_OBJ_RECTANGLE,
    VX_OBJ_COORDINATES,
};

#endif

