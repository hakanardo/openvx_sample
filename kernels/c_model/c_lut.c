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

// nodeless version of the TableLookup kernel
vx_status vxTableLookup(vx_image src, vx_lut lut, vx_image dst)
{
    vx_enum type = 0;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    void *src_base = NULL, *dst_base = NULL, *lut_ptr = NULL;
    vx_uint32 y = 0, x = 0;
    vx_size count = 0;
    vx_status status = VX_SUCCESS;

    vxQueryLUT(lut, VX_LUT_ATTRIBUTE_TYPE, &type, sizeof(type));
    vxQueryLUT(lut, VX_LUT_ATTRIBUTE_COUNT, &count, sizeof(count));
    status = vxGetValidRegionImage(src, &rect);
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
    status |= vxAccessLUT(lut, &lut_ptr, VX_READ_ONLY);

    for (y = 0; (y < src_addr.dim_y) && (status == VX_SUCCESS); y++)
    {
        for (x = 0; x < src_addr.dim_x; x++)
        {
            if (type == VX_TYPE_UINT8)
            {
                vx_uint8 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 *dst_ptr = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                vx_uint8 *lut_tmp = (vx_uint8 *)lut_ptr;
                if ((*src_ptr) < count)
                {
                    *dst_ptr = lut_tmp[(*src_ptr)];
                }
            }
            else if (type == VX_TYPE_INT16)
            {
                vx_int16 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_int16 *dst_ptr = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                vx_int16 *lut_tmp = (vx_int16 *)lut_ptr;
                if (((*src_ptr) < count) && ((*src_ptr) >= 0))
                {
                    *dst_ptr = lut_tmp[(*src_ptr)];
                }
            }
        }
    }

    status |= vxCommitLUT(lut, lut_ptr);
    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}

