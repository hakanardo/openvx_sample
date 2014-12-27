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

// nodeless version of the XXXX kernel
vx_status vxIntegralImage(vx_image src, vx_image dst)
{
    vx_uint32 y, x;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_rectangle_t rect;

    vx_status status = VX_SUCCESS;
    status = vxGetValidRegionImage(src, &rect);
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    for (y = 0; (y < src_addr.dim_y) && (status == VX_SUCCESS); y++)
    {
        vx_uint8 *pixels = vxFormatImagePatchAddress2d(src_base, 0, y, &src_addr);
        vx_uint32 *sums = vxFormatImagePatchAddress2d(dst_base, 0, y, &dst_addr);

        if (y == 0)
        {
            sums[0] = pixels[0];
            for (x = 1; x < src_addr.dim_x; x++)
            {
                sums[x] = sums[x-1] + pixels[x];
            }
        }
        else
        {
            vx_uint32 *prev_sums = vxFormatImagePatchAddress2d(dst_base, 0, y-1, &dst_addr);
            sums[0] = prev_sums[0] + pixels[0];
            for (x = 1; x < src_addr.dim_x; x++)
            {
                sums[x] = pixels[x] + sums[x-1] + prev_sums[x] - prev_sums[x-1];
            }
        }
    }
    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}

