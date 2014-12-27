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

vx_status example_conv(vx_context context)
{
    //! [assign]
    // A horizontal Scharr gradient operator with different scale.
    vx_int16 gx[3][3] = {
        {  3, 0, -3},
        { 10, 0,-10},
        {  3, 0, -3},
    };
    vx_uint32 scale = 9;
    vx_convolution scharr_x = vxCreateConvolution(context, 3, 3);
    vxAccessConvolutionCoefficients(scharr_x, NULL);
    vxCommitConvolutionCoefficients(scharr_x, (vx_int16*)gx);
    vxSetConvolutionAttribute(scharr_x, VX_CONVOLUTION_ATTRIBUTE_SCALE, &scale, sizeof(scale));
    //! [assign]
    vxReleaseConvolution(&scharr_x);
    return VX_SUCCESS;
}
