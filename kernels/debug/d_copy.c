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

#include <debug_k.h>

// nodeless version of the CopyImage kernel
vx_status vxCopyImage(vx_image input, vx_image output)
{
    vx_status status = VX_SUCCESS; // assume success until an error occurs.
    vx_uint32 p = 0, y = 0, len = 0;
    vx_size planes = 0;
    void *src, *dst;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_rectangle_t rect;

    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
    status |= vxGetValidRegionImage(input, &rect);
    for (p = 0; p < planes && status == VX_SUCCESS; p++)
    {
        status = VX_SUCCESS;
        src = dst = NULL;
        status |= vxAccessImagePatch(input, &rect, p, &src_addr, &src, VX_READ_ONLY);
        status |= vxAccessImagePatch(output, &rect, p, &dst_addr, &dst, VX_WRITE_ONLY);
        for (y = 0; y < src_addr.dim_y && status == VX_SUCCESS; y+=src_addr.step_y)
        {
            /*
             * in the case where the secondary planes are subsampled, the
             * scale will skip over the lines that are repeated.
             */
            vx_uint8 *srcp = vxFormatImagePatchAddress2d(src, 0, y, &src_addr);
            vx_uint8 *dstp = vxFormatImagePatchAddress2d(dst, 0, y, &dst_addr);
            len = (src_addr.stride_x * src_addr.dim_x * src_addr.scale_x) / VX_SCALE_UNITY;
            memcpy(dstp, srcp, len);
        }
        if (status == VX_SUCCESS) {
            status |= vxCommitImagePatch(input, NULL, p, &src_addr, src);
            status |= vxCommitImagePatch(output, &rect, p, &dst_addr, dst);
        }
    }

    return status;
}


// nodeless version of the CopyArray kernel
vx_status vxCopyArray(vx_array src, vx_array dst)
{
    vx_status status = VX_SUCCESS; // assume success until an error occurs.
    vx_size src_num_items = 0, dst_capacity = 0, src_stride = 0;
    void *srcp = NULL;

    status = VX_SUCCESS;
    status |= vxQueryArray(src, VX_ARRAY_ATTRIBUTE_NUMITEMS, &src_num_items, sizeof(src_num_items));
    status |= vxQueryArray(dst, VX_ARRAY_ATTRIBUTE_CAPACITY, &dst_capacity, sizeof(dst_capacity));
    if (status == VX_SUCCESS)
    {
        status |= vxAccessArrayRange(src, 0, src_num_items, &src_stride, &srcp, VX_READ_ONLY);

        if (src_num_items <= dst_capacity && status == VX_SUCCESS)
        {
            status |= vxTruncateArray(dst, 0);
            status |= vxAddArrayItems(dst, src_num_items, srcp, src_stride);
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        status |= vxCommitArrayRange(src, 0, 0, srcp);
    }

    return status;
}

