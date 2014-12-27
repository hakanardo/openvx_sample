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
#include <stdlib.h>


// helpers
static int vx_uint8_compare(const void *p1, const void *p2)
{
    vx_uint8 a = *(vx_uint8 *)p1;
    vx_uint8 b = *(vx_uint8 *)p2;
    if (a > b)
        return 1;
    else if (a == b)
        return 0;
    else
        return -1;
}


// nodeless version of the Median3x3 kernel
vx_status vxMedian3x3(vx_image src, vx_image dst, vx_border_mode_t *borders)
{
    vx_uint32 y, x;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_rectangle_t rect;
    vx_uint32 low_x = 0, low_y = 0, high_x, high_y;

    vx_status status = vxGetValidRegionImage(src, &rect);
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    high_x = src_addr.dim_x;
    high_y = src_addr.dim_y;

    if (borders->mode == VX_BORDER_MODE_UNDEFINED)
    {
        ++low_x; --high_x;
        ++low_y; --high_y;
        vxAlterRectangle(&rect, 1, 1, -1, -1);
    }

    for (y = low_y; (y < high_y) && (status == VX_SUCCESS); y++)
    {
        for (x = low_x; x < high_x; x++)
        {
            vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            vx_uint8 values[9];

            vxReadRectangle(src_base, &src_addr, borders, VX_DF_IMAGE_U8, x, y, 1, 1, values);

            qsort(values, dimof(values), sizeof(vx_uint8), vx_uint8_compare);
            *dst = values[4]; /* pick the middle value */
        }
    }

    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}


// nodeless version of the Box3x3 kernel
static vx_int16 box[3][3] = {
    {1, 1, 1},
    {1, 1, 1},
    {1, 1, 1},
};

vx_status vxBox3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return vxConvolution3x3(src, dst, box, bordermode);
}


// nodeless version of the Gaussian3x3 kernel
static vx_int16 gaussian[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1},
};

vx_status vxGaussian3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return vxConvolution3x3(src, dst, gaussian, bordermode);
}

