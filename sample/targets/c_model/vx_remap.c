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

/*!
 * \file
 * \brief The Remap Kernel
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>
#include <vx_internal.h>

static vx_bool read_pixel(void* base, vx_imagepatch_addressing_t* addr,
    vx_float32 x, vx_float32 y, const vx_border_mode_t* borders, vx_uint8* pixel)
{
    vx_bool out_of_bounds = (x < 0 || y < 0 || x >= addr->dim_x || y >= addr->dim_y);
    vx_uint32 bx, by;
    vx_uint8 *bpixel;
    if (out_of_bounds)
    {
        if (borders->mode == VX_BORDER_MODE_UNDEFINED)
            return vx_false_e;
        if (borders->mode == VX_BORDER_MODE_CONSTANT)
        {
            *pixel = borders->constant_value;
            return vx_true_e;
        }
    }

    // bounded x/y
    bx = x < 0 ? 0 : x >= addr->dim_x ? addr->dim_x - 1 : (vx_uint32)x;
    by = y < 0 ? 0 : y >= addr->dim_y ? addr->dim_y - 1 : (vx_uint32)y;

    bpixel = vxFormatImagePatchAddress2d(base, bx, by, addr);
    *pixel = *bpixel;

    return vx_true_e;
}


static vx_status VX_CALLBACK vxRemapKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 4)
    {
        vx_image src_image = (vx_image)parameters[0];
        vx_remap table = (vx_remap)parameters[1];
        vx_scalar stype = (vx_scalar)parameters[2];
        vx_image dst_image = (vx_image)parameters[3];
        vx_enum policy = 0;
        void *src_base = NULL;
        void *dst_base = NULL;
        vx_imagepatch_addressing_t src_addr, dst_addr;
        vx_uint32 y = 0u, x = 0u, width = 0u, height = 0u;
        vx_rectangle_t src_rect;
        vx_rectangle_t dst_rect;
        vx_border_mode_t borders;

        vxAccessScalarValue(stype, &policy);
        vxQueryImage(src_image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
        vxQueryImage(src_image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
        src_rect.start_x = src_rect.start_y = 0;
        src_rect.end_x = width;
        src_rect.end_y = height;
        vxQueryImage(dst_image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
        vxQueryImage(dst_image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
        dst_rect.start_x = dst_rect.start_y = 0;
        dst_rect.end_x = width;
        dst_rect.end_y = height;

        vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

        status = VX_SUCCESS;
        status |= vxAccessImagePatch(src_image, &src_rect, 0, &src_addr, &src_base, VX_READ_ONLY);
        status |= vxAccessImagePatch(dst_image, &dst_rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
        if (status == VX_SUCCESS)
        {
            /* iterate over the destination image */
            for (y = 0u; y < height; y++)
            {
                for (x = 0u; x < width; x++)
                {
                    vx_float32 src_x = 0.0f;
                    vx_float32 src_y = 0.0f;
                    vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                    status = vxGetRemapPoint(table, x, y, &src_x, &src_y);
                    //printf("Remapping %lf,%lf to %lu,%lu\n",src_x, src_y, x, y);
                    if (policy == VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR)
                    {
                        /* this rounds then truncates the decimal side */
                        read_pixel(src_base, &src_addr, src_x + 0.5f, src_y + 0.5f, &borders, dst);
                    }
                    else if (policy == VX_INTERPOLATION_TYPE_BILINEAR)
                    {
                        vx_uint8 tl = 0;
                        vx_uint8 tr = 0;
                        vx_uint8 bl = 0;
                        vx_uint8 br = 0;
                        vx_float32 xf = floorf(src_x);
                        vx_float32 yf = floorf(src_y);
                        vx_float32 dx = src_x - xf;
                        vx_float32 dy = src_y - yf;
                        vx_float32 a[] =
                        {
                            (1.0f - dx) * (1.0f - dy),
                            (1.0f - dx) * (dy),
                            (dx)* (1.0f - dy),
                            (dx)* (dy),
                        };
                        vx_bool defined = vx_true_e;
                        defined &= read_pixel(src_base, &src_addr, xf + 0, yf + 0, &borders, &tl);
                        defined &= read_pixel(src_base, &src_addr, xf + 1, yf + 0, &borders, &tr);
                        defined &= read_pixel(src_base, &src_addr, xf + 0, yf + 1, &borders, &bl);
                        defined &= read_pixel(src_base, &src_addr, xf + 1, yf + 1, &borders, &br);
                        if (defined)
                        {
                            *dst = (vx_uint8)(a[0]*tl + a[2]*tr + a[1]*bl + a[3]*br);
                        }
                     }
                }
            }
        }
        status |= vxCommitImagePatch(src_image, NULL, 0, &src_addr, src_base);
        status |= vxCommitImagePatch(dst_image, &dst_rect, 0, &dst_addr, dst_base);
        status = VX_SUCCESS;
    }
    return status;
}

static vx_status VX_CALLBACK vxRemapInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_U8)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_remap table;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &table, sizeof(table));
            if (table)
            {
                /* \todo what are we checking? */
                status = VX_SUCCESS;
                vxReleaseRemap(&table);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar scalar = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum stype = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
                if (stype == VX_TYPE_ENUM)
                {
                    vx_enum policy = 0;
                    vxAccessScalarValue(scalar, &policy);
                    if ((policy == VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR) ||
                        (policy == VX_INTERPOLATION_TYPE_BILINEAR))
                    {
                        status = VX_SUCCESS;
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_VALUE;
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_TYPE;
                }
                vxReleaseScalar(&scalar);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxRemapOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter tbl_param = vxGetParameterByIndex(node, 1);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);
        if (src_param && dst_param && tbl_param)
        {
            vx_image src = 0;
            vx_image dst = 0;
            vx_remap tbl = 0;
            vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
            vxQueryParameter(dst_param, VX_PARAMETER_ATTRIBUTE_REF, &dst, sizeof(dst));
            vxQueryParameter(tbl_param, VX_PARAMETER_ATTRIBUTE_REF, &tbl, sizeof(tbl));
            if ((src) && (dst) && (tbl))
            {
                vx_uint32 w1 = 0, h1 = 0;
                vx_uint32 w2 = 0, h2 = 0;
                vx_uint32 w3 = 0, h3 = 0;

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &w1, sizeof(w1));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &h1, sizeof(h1));
                vxQueryRemap(tbl, VX_REMAP_ATTRIBUTE_SOURCE_WIDTH, &w2, sizeof(w2));
                vxQueryRemap(tbl, VX_REMAP_ATTRIBUTE_SOURCE_HEIGHT, &h2, sizeof(h2));
                vxQueryRemap(tbl, VX_REMAP_ATTRIBUTE_DESTINATION_WIDTH, &w3, sizeof(w3));
                vxQueryRemap(tbl, VX_REMAP_ATTRIBUTE_DESTINATION_HEIGHT, &h3, sizeof(h3));

                if ((w1 == w2) && (h1 == h2))
                {
                    /* fill in the meta data with the attributes so that the checker will pass */
                    ptr->type = VX_TYPE_IMAGE;
                    ptr->dim.image.format = VX_DF_IMAGE_U8;
                    ptr->dim.image.width = w3;
                    ptr->dim.image.height = h3;
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&src);
                vxReleaseRemap(&tbl);
                vxReleaseImage(&dst);
            }
            vxReleaseParameter(&src_param);
            vxReleaseParameter(&tbl_param);
            vxReleaseParameter(&dst_param);
        }
    }
    return status;
}

static vx_param_description_t remap_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_REMAP, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_t remap_kernel = {
    VX_KERNEL_REMAP,
    "org.khronos.openvx.remap",
    vxRemapKernel,
    remap_kernel_params, dimof(remap_kernel_params),
    vxRemapInputValidator,
    vxRemapOutputValidator,
    NULL,
    NULL,
};
