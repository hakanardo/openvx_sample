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
 * \brief Elementwise binary norm kernel (Extras)
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>
#include <math.h>
#include <stdlib.h>

static vx_status vxNorm(vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_uint32 y, x;
    vx_df_image format = 0;
    vx_uint8 *dst_base   = NULL;
    vx_int16 *src_base_x = NULL;
    vx_int16 *src_base_y = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr_x, src_addr_y;
    vx_rectangle_t rect;
    vx_enum norm_type_value;

    vxAccessScalarValue(norm_type, &norm_type_value);
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    vxGetValidRegionImage(input_x, &rect);
    status = VX_SUCCESS;
    status |= vxAccessImagePatch(input_x, &rect, 0, &src_addr_x, (void **)&src_base_x, VX_READ_ONLY);
    status |= vxAccessImagePatch(input_y, &rect, 0, &src_addr_y, (void **)&src_base_y, VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, (void **)&dst_base, VX_WRITE_ONLY);
    for (y = 0; y < src_addr_x.dim_y; y++)
    {
        for (x = 0; x < src_addr_x.dim_x; x++)
        {
            vx_int16 *in_x = vxFormatImagePatchAddress2d(src_base_x, x, y, &src_addr_x);
            vx_int16 *in_y = vxFormatImagePatchAddress2d(src_base_y, x, y, &src_addr_y);
            vx_uint16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            vx_uint32 value;

            if (norm_type_value == VX_NORM_L1)
            {
                value = abs(in_x[0]) + abs(in_y[0]);
            }
            else
            {
                vx_uint32 squares[2] = { (vx_int32)in_x[0]*in_x[0], (vx_int32)in_y[0]*in_y[0] };
#ifdef _MSC_VER
                value = 0.5f + sqrt(squares[0] + squares[1]);
#else
                value = lrintf(sqrt(squares[0] + squares[1]));
#endif
            }

            *dst = (vx_uint16)(value > UINT16_MAX ? UINT16_MAX : value);
        }
    }
    status |= vxCommitImagePatch(input_x, 0, 0, &src_addr_x, src_base_x);
    status |= vxCommitImagePatch(input_y, 0, 0, &src_addr_y, src_base_y);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);
    return status;
}

static vx_status VX_CALLBACK vxNormKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 4)
    {
        vx_image input_x = (vx_image)parameters[0];
        vx_image input_y = (vx_image)parameters[1];
        vx_scalar norm_type = (vx_scalar)parameters[2];
        vx_image output = (vx_image)parameters[3];
        return vxNorm(input_x, input_y, norm_type, output);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxNormInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = vxGetParameterByIndex(node, index);

    if (index == 0 || index == 1)
    {
        vx_image input = 0;

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_S16)
            {
                if (index == 0)
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    vx_parameter param0 = vxGetParameterByIndex(node, index);
                    vx_image input0 = 0;

                    vxQueryParameter(param0, VX_PARAMETER_ATTRIBUTE_REF, &input0, sizeof(input0));
                    if (input0)
                    {
                        vx_uint32 width0 = 0, height0 = 0, width1 = 0, height1 = 0;
                        vxQueryImage(input0, VX_IMAGE_ATTRIBUTE_WIDTH, &width0, sizeof(width0));
                        vxQueryImage(input0, VX_IMAGE_ATTRIBUTE_HEIGHT, &height0, sizeof(height0));
                        vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width1, sizeof(width1));
                        vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height1, sizeof(height1));

                        if (width0 == width1 && height0 == height1)
                            status = VX_SUCCESS;
                        vxReleaseImage(&input0);
                    }
                    vxReleaseParameter(&param0);
                }
            }
            vxReleaseImage(&input);
        }
    }
    if (index == 2)
    {
        vx_scalar norm_type;
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &norm_type, sizeof(norm_type));

        if (norm_type)
        {
            vx_enum type;
            vxQueryScalar(norm_type, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));

            if (type == VX_TYPE_ENUM)
            {
                vx_enum value;
                vxAccessScalarValue(norm_type, &value);
                if (value == VX_NORM_L1 || value == VX_NORM_L2)
                    status = VX_SUCCESS;
            }

            vxReleaseScalar(&norm_type);
        }
    }

    vxReleaseParameter(&param);

    return status;
}

static vx_status VX_CALLBACK vxNormOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        vx_parameter param0  = vxGetParameterByIndex(node, 0);
        if (param0)
        {
            vx_image input = 0;
            vxQueryParameter(param0, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_U16;
                status = VX_SUCCESS;
                status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(format));
                vxReleaseImage(&input);
            }
            vxReleaseParameter(&param0);
        }
    }
    return status;
}

static vx_param_description_t norm_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t norm_kernel = {
    VX_KERNEL_EXTRAS_ELEMENTWISE_NORM,
    "org.khronos.extra.elementwise_norm",
    vxNormKernel,
    norm_kernel_params, dimof(norm_kernel_params),
    vxNormInputValidator,
    vxNormOutputValidator,
    NULL,
    NULL,
};
