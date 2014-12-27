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

// nodeless version of the Accumulate kernel
vx_status vxAccumulate(vx_image input, vx_image accum)
{
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base = NULL;
    void *src_base = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr;
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;

    status = vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &src_addr, (void **)&src_base,VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(accum, &rect, 0, &dst_addr, (void **)&dst_base,VX_READ_AND_WRITE);
    width = src_addr.dim_x;
    height = src_addr.dim_y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            vx_uint8 *srcp = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            vx_int16 *dstp = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            vx_int32 res = ((vx_int32)(*dstp)) + (vx_int32)(*srcp);
            if (res > INT16_MAX) // saturate to S16
                res = INT16_MAX;
            *dstp = (vx_int16)(res);
        }
    }
    status |= vxCommitImagePatch(input, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(accum, &rect, 0, &dst_addr, dst_base);

    return status;
}

// nodeless version of the AccumulateWeighted kernel
vx_status vxAccumulateWeighted(vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base = NULL;
    void *src_base = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr;
    vx_rectangle_t rect;
    vx_float32 alpha = 0.0f;
    vx_status status  = VX_SUCCESS;

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    rect.start_x = rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;
    status |= vxAccessImagePatch(input, &rect, 0, &src_addr, (void **)&src_base,VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(accum, &rect, 0, &dst_addr, (void **)&dst_base,VX_READ_AND_WRITE);
    status |= vxAccessScalarValue(scalar, &alpha);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            vx_uint8 *srcp = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            vx_uint8 *dstp = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            *dstp = (vx_uint16)(((1 - alpha) * (*dstp)) + ((alpha) * (vx_uint16)(*srcp)));
        }
    }
    status |= vxCommitImagePatch(input, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(accum, &rect, 0, &dst_addr, dst_base);

    return status;
}

// nodeless version of the AccumulateSquare kernel
vx_status vxAccumulateSquare(vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base = NULL;
    void *src_base = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr;
    vx_rectangle_t rect;
    vx_uint32 shift = 0u;
    vx_status status = VX_SUCCESS;

    vxAccessScalarValue(scalar, &shift);
    status = vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &src_addr, (void **)&src_base,VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(accum, &rect, 0, &dst_addr, (void **)&dst_base,VX_READ_AND_WRITE);
    width = src_addr.dim_x;
    height = src_addr.dim_y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            vx_uint8 *srcp = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            vx_int16 *dstp = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            vx_int32 res = ((vx_int32)(*srcp) * (vx_int32)(*srcp));
            res = ((vx_int32)*dstp) + (res >> shift);
            if (res > INT16_MAX) // saturate to S16
                res = INT16_MAX;
            *dstp = (vx_int16)(res);
        }
    }
    status |= vxCommitImagePatch(input, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(accum, &rect, 0, &dst_addr, dst_base);

    return status;
}

