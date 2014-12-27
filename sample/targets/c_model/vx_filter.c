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
 * \brief The Filter Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>


static vx_status VX_CALLBACK vxMedian3x3Kernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 2)
    {
        vx_border_mode_t bordermode;
        vx_image src = (vx_image)parameters[0];
        vx_image dst = (vx_image)parameters[1];
        status = vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));
        if (status == VX_SUCCESS)
        {
            status = vxMedian3x3(src, dst, &bordermode);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxBox3x3Kernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 2)
    {
        vx_border_mode_t bordermode;
        vx_image src  = (vx_image)parameters[0];
        vx_image dst = (vx_image)parameters[1];
        status = vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));
        if (status == VX_SUCCESS)
        {
            status = vxBox3x3(src, dst, &bordermode);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxGaussian3x3Kernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 2)
    {
        vx_border_mode_t bordermode;
        vx_image src  = (vx_image)parameters[0];
        vx_image dst = (vx_image)parameters[1];
        status = vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));
        if (status == VX_SUCCESS)
        {
            status = vxGaussian3x3(src, dst, &bordermode);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxFilterInputValidator(vx_node node, vx_uint32 index)
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
    return status;
}

static vx_status VX_CALLBACK vxFilterOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0); /* we reference the input image */
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_uint32 width = 0, height = 0;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = VX_DF_IMAGE_U8;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                status = VX_SUCCESS;
                vxReleaseImage(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_param_description_t filter_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t box3x3_kernel = {
    VX_KERNEL_BOX_3x3,
    "org.khronos.openvx.box3x3:default",
    vxBox3x3Kernel,
    filter_kernel_params, dimof(filter_kernel_params),
    vxFilterInputValidator,
    vxFilterOutputValidator,
};

vx_kernel_description_t box3x3_kernel_2 = {
    VX_KERNEL_BOX_3x3,
    "org.khronos.openvx.box3x3:duplicate",
    vxBox3x3Kernel,
    filter_kernel_params, dimof(filter_kernel_params),
    vxFilterInputValidator,
    vxFilterOutputValidator,
};

vx_kernel_description_t median3x3_kernel = {
    VX_KERNEL_MEDIAN_3x3,
    "org.khronos.openvx.median3x3",
    vxMedian3x3Kernel,
    filter_kernel_params, dimof(filter_kernel_params),
    vxFilterInputValidator,
    vxFilterOutputValidator,
};

vx_kernel_description_t gaussian3x3_kernel = {
    VX_KERNEL_GAUSSIAN_3x3,
    "org.khronos.openvx.gaussian3x3",
    vxGaussian3x3Kernel,
    filter_kernel_params, dimof(filter_kernel_params),
    vxFilterInputValidator,
    vxFilterOutputValidator,
};
