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
 * \brief The Custom Convolution Kernel
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>

#if (C_MAX_CONVOLUTION_DIM != VX_INT_MAX_CONVOLUTION_DIM)
#if defined(_WIN32)
#pragma error("C Model does not support VX required Convolution Size")
#elif defined(__GNUC__)
#error "C Model does not support VX required Convolution Size"
#endif
#endif

static vx_status VX_CALLBACK vxConvolveKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 3)
    {
        vx_border_mode_t bordermode;
        vx_image       src  = (vx_image)parameters[0];
        vx_convolution conv = (vx_convolution)parameters[1];
        vx_image       dst  = (vx_image)parameters[2];
        status = vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));
        if (status == VX_SUCCESS)
        {
            status = vxConvolve(src, conv, dst, &bordermode);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxConvolveInputValidator(vx_node node, vx_uint32 index)
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
            vx_uint32 width = 0, height = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if ((width > VX_INT_MAX_CONVOLUTION_DIM) &&
                (height > VX_INT_MAX_CONVOLUTION_DIM) &&
                ((format == VX_DF_IMAGE_U8)
#if defined(EXPERIMENTAL_USE_S16)
                 || (format == VX_DF_IMAGE_S16)
#endif
                 ))
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    if (index == 1)
    {
        vx_convolution conv = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &conv, sizeof(conv));
        if (conv)
        {
            vx_df_image dims[2] = {0,0};
            vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_COLUMNS, &dims[0], sizeof(dims[0]));
            vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_ROWS, &dims[1], sizeof(dims[1]));
            if ((dims[0] <= VX_INT_MAX_CONVOLUTION_DIM) &&
                (dims[1] <= VX_INT_MAX_CONVOLUTION_DIM))
            {
                status = VX_SUCCESS;
            }
            vxReleaseConvolution(&conv);
        }
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_status VX_CALLBACK vxConvolveOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter params[2] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, index),
        };
        if (params[0] && params[1])
        {
            vx_image input = 0;
            vx_image output = 0;
            vxQueryParameter(params[0], VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            vxQueryParameter(params[1], VX_PARAMETER_ATTRIBUTE_REF, &output, sizeof(output));
            if (input && output)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = 0;
                vx_df_image output_format = 0;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

                vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &output_format, sizeof(output_format));

                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = output_format == VX_DF_IMAGE_U8 ? VX_DF_IMAGE_U8 : VX_DF_IMAGE_S16;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                status = VX_SUCCESS;

                vxReleaseImage(&input);
                vxReleaseImage(&output);
            }
            vxReleaseParameter(&params[0]);
            vxReleaseParameter(&params[1]);
        }
    }
    return status;
}

static vx_param_description_t convolution_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_CONVOLUTION, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t convolution_kernel = {
    VX_KERNEL_CUSTOM_CONVOLUTION,
    "org.khronos.openvx.custom_convolution",
    vxConvolveKernel,
    convolution_kernel_params, dimof(convolution_kernel_params),
    vxConvolveInputValidator,
    vxConvolveOutputValidator,
    NULL,
    NULL,
};

