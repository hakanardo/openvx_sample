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

// nodeless version of the AbsDiff kernel
vx_status vxAbsDiff(vx_image in1, vx_image in2, vx_image output)
{
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base   = NULL;
    void *src_base[2] = {NULL, NULL};
    vx_imagepatch_addressing_t dst_addr, src_addr[2];
    vx_rectangle_t rect, r_in1, r_in2;
    vx_df_image format;
    vx_status status = VX_SUCCESS;

    vxQueryImage(in1, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    status  = vxGetValidRegionImage(in1, &r_in1);
    status |= vxGetValidRegionImage(in2, &r_in2);
    vxFindOverlapRectangle(&r_in1, &r_in2, &rect);
    //printf("%s Rectangle = {%u,%u x %u,%u}\n",__FUNCTION__, rect.start_x, rect.start_y, rect.end_x, rect.end_y);
    status |= vxAccessImagePatch(in1, &rect, 0, &src_addr[0], (void **)&src_base[0],VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(in2, &rect, 0, &src_addr[1], (void **)&src_base[1],VX_READ_AND_WRITE);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, (void **)&dst_base,VX_READ_AND_WRITE);
    height = src_addr[0].dim_y;
    width = src_addr[0].dim_x;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (format == VX_DF_IMAGE_U8)
            {
                vx_uint8 *src[2] = {
                    vxFormatImagePatchAddress2d(src_base[0], x, y, &src_addr[0]),
                    vxFormatImagePatchAddress2d(src_base[1], x, y, &src_addr[1]),
                };
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (*src[0] > *src[1])
                    *dst = *src[0] - *src[1];
                else
                    *dst = *src[1] - *src[0];
            }
            else if (format == VX_DF_IMAGE_S16)
            {
                vx_int16 *src[2] = {
                    vxFormatImagePatchAddress2d(src_base[0], x, y, &src_addr[0]),
                    vxFormatImagePatchAddress2d(src_base[1], x, y, &src_addr[1]),
                };
                vx_uint16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (*src[0] > *src[1])
                    *dst = *src[0] - *src[1];
                else
                    *dst = *src[1] - *src[0];
            }
            else if (format == VX_DF_IMAGE_U16)
            {
                vx_uint16 *src[2] = {
                    vxFormatImagePatchAddress2d(src_base[0], x, y, &src_addr[0]),
                    vxFormatImagePatchAddress2d(src_base[1], x, y, &src_addr[1]),
                };
                vx_uint16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (*src[0] > *src[1])
                    *dst = *src[0] - *src[1];
                else
                    *dst = *src[1] - *src[0];
            }
        }
    }
    status |= vxCommitImagePatch(in1, NULL, 0, &src_addr[0], src_base[0]);
    status |= vxCommitImagePatch(in2, NULL, 0, &src_addr[1], src_base[1]);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);

    return status;
}

