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
#include <VX/vx.h>

// nodeless version of the Convolve kernel
vx_status vxConvolve(vx_image src, vx_convolution conv, vx_image dst, vx_border_mode_t *bordermode)
{
    vx_int32 y, x, i;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_rectangle_t rect;
    vx_size conv_width, conv_height;
    vx_int32 conv_radius_x, conv_radius_y;
    vx_int16 conv_mat[C_MAX_CONVOLUTION_DIM * C_MAX_CONVOLUTION_DIM] = {0};
    vx_int32 sum = 0, value = 0;
    vx_uint32 scale = 1;
    vx_df_image src_format = 0;
    vx_df_image dst_format = 0;
    vx_status status  = VX_SUCCESS;
    vx_int32 low_x, low_y, high_x, high_y;

    status |= vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &src_format, sizeof(src_format));
    status |= vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_FORMAT, &dst_format, sizeof(dst_format));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_COLUMNS, &conv_width, sizeof(conv_width));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_ROWS, &conv_height, sizeof(conv_height));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_SCALE, &scale, sizeof(scale));
    conv_radius_x = (vx_int32)conv_width / 2;
    conv_radius_y = (vx_int32)conv_height / 2;
    status |= vxAccessConvolutionCoefficients(conv, conv_mat);
    status |= vxGetValidRegionImage(src, &rect);
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    if (bordermode->mode == VX_BORDER_MODE_UNDEFINED)
    {
        low_x = conv_radius_x;
        high_x = ((src_addr.dim_x >= (vx_uint32)conv_radius_x) ? src_addr.dim_x - conv_radius_x : 0);
        low_y = conv_radius_y;
        high_y = ((src_addr.dim_y >= (vx_uint32)conv_radius_y) ? src_addr.dim_y - conv_radius_y : 0);
        vxAlterRectangle(&rect, conv_radius_x, conv_radius_y, -conv_radius_x, -conv_radius_y);
    }
    else
    {
        low_x = 0;
        high_x = src_addr.dim_x;
        low_y = 0;
        high_y = src_addr.dim_y;
    }

    for (y = low_y; y < high_y; ++y)
    {
        for (x = low_x; x < high_x; ++x)
        {
            sum = 0;

            if (src_format == VX_DF_IMAGE_U8)
            {
                vx_uint8 slice[C_MAX_CONVOLUTION_DIM * C_MAX_CONVOLUTION_DIM] = {0};

                vxReadRectangle(src_base, &src_addr, bordermode, src_format, x, y, conv_radius_x, conv_radius_y, slice);

                for (i = 0; i < conv_width * conv_height; ++i)
                    sum += conv_mat[conv_width * conv_height - 1 - i] * slice[i];
            }
            else if (src_format == VX_DF_IMAGE_S16)
            {
                vx_int16 slice[C_MAX_CONVOLUTION_DIM * C_MAX_CONVOLUTION_DIM] = {0};

                vxReadRectangle(src_base, &src_addr, bordermode, src_format, x, y, conv_radius_x, conv_radius_y, slice);

                for (i = 0; i < conv_width * conv_height; ++i)
                    sum += conv_mat[conv_width * conv_height - 1 - i] * slice[i];
            }

            value = sum / (vx_int32) scale;

            if (dst_format == VX_DF_IMAGE_U8)
            {
                vx_uint8 *dstp = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (value < 0) *dstp = 0;
                else if (value > UINT8_MAX) *dstp = UINT8_MAX;
                else *dstp = value;
            }
            else if (dst_format == VX_DF_IMAGE_S16)
            {
                vx_int16 *dstp = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (value < INT16_MIN) *dstp = INT16_MIN;
                else if (value > INT16_MAX) *dstp = INT16_MAX;
                else *dstp = value;
            }
        }
    }

    status |= vxCommitConvolutionCoefficients(conv, NULL);
    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}

