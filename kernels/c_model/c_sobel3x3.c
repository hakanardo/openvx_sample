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

#include <c_model.h>

static vx_int16 sobel_x[3][3] = {
    {-1, 0, +1},
    {-2, 0, +2},
    {-1, 0, +1},
};

static vx_int16 sobel_y[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    {+1, +2, +1},
};

// nodeless version of the Sobel3x3 kernel
vx_status vxSobel3x3(vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode)
{
    if (grad_x) {
        vx_status status = vxConvolution3x3(input, grad_x, sobel_x, bordermode);
        if (status != VX_SUCCESS) return status;
    }

    if (grad_y) {
        vx_status status = vxConvolution3x3(input, grad_y, sobel_y, bordermode);
        if (status != VX_SUCCESS) return status;
    }

    return VX_SUCCESS;
}
