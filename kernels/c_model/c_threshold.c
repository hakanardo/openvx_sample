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
#include <vx_debug.h>

// nodeless version of the Threshold kernel
vx_status vxThreshold(vx_image src_image, vx_threshold threshold, vx_image dst_image)
{
    vx_enum type = 0;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    void *src_base = NULL, *dst_base = NULL;
    vx_uint32 y = 0, x = 0;
    vx_int32 value = 0, lower = 0, upper = 0;
    vx_status status = VX_FAILURE;

    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_TYPE, &type, sizeof(type));
    if (type == VX_THRESHOLD_TYPE_BINARY)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE, &value, sizeof(value));
    }
    else if (type == VX_THRESHOLD_TYPE_RANGE)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
        vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));
    }
    status = vxGetValidRegionImage(src_image, &rect);
    status |= vxAccessImagePatch(src_image, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst_image, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
    VX_PRINT(VX_ZONE_INFO, "threshold = %u\n", value);
    for (y = 0; y < src_addr.dim_y; y++)
    {
        for (x = 0; x < src_addr.dim_x; x++)
        {
            vx_uint8 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            vx_uint8 *dst_ptr = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);

            if (type == VX_THRESHOLD_TYPE_BINARY)
            {
                if (*src_ptr > value)
                    *dst_ptr = 255;
                else
                    *dst_ptr = 0;
            }
            if (type == VX_THRESHOLD_TYPE_RANGE)
            {
                if (*src_ptr > upper)
                    *dst_ptr = 0;
                else if (*src_ptr < lower)
                    *dst_ptr = 0;
                else
                    *dst_ptr = 255;
            }
        }
    }

    status |= vxCommitImagePatch(src_image, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst_image, &rect, 0, &dst_addr, dst_base);

    return status;
}

