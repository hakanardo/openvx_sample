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

#include <VX/vx.h>
#include <VX/vxu.h>

vx_status vx_example_warp_affine(vx_context context, vx_image src, vx_image dst)
{
    vx_float32 a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f, e = 0.0f , f = 0.0f;
    //! [warp affine]
    // x0 = a x + b y + c;
    // y0 = d x + e y + f;
    vx_float32 mat[3][2] = {
        {a, d}, // 'x' coefficients
        {b, e}, // 'y' coefficients
        {c, f}, // 'offsets'
    };
    vx_matrix matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 2, 3);
    vxAccessMatrix(matrix, NULL);
    vxCommitMatrix(matrix, mat);
    //! [warp affine]
    return vxuWarpAffine(context, src, matrix, VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, dst);
}

vx_status vx_example_warp_perspective(vx_context context, vx_image src, vx_image dst)
{
    vx_float32 a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f, e = 0.0f, f = 0.0f, g = 1.0f, h = 0.0f, i = 0.0f;
    //! [warp perspective]
    // x0 = a x + b y + c;
    // y0 = d x + e y + f;
    // z0 = g x + h y + i;
    vx_float32 mat[3][3] = {
        {a, d, g}, // 'x' coefficients
        {b, e, h}, // 'y' coefficients
        {c, f, i}, // 'offsets'
    };
    vx_matrix matrix = vxCreateMatrix(context, VX_TYPE_FLOAT32, 3, 3);
    vxAccessMatrix(matrix, NULL);
    vxCommitMatrix(matrix, mat);
    //! [warp perspective]
    return vxuWarpPerspective(context, src, matrix, VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR, dst);
}
