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
 * \brief The Gradient Phase Kernel
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>
#include <math.h>

static vx_status VX_CALLBACK vxPhaseKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 3)
    {
        vx_image grad_x = (vx_image)parameters[0];
        vx_image grad_y = (vx_image)parameters[1];
        vx_image output = (vx_image)parameters[2];
        return vxPhase(grad_x, grad_y, output);
    }
    return status;
}

static vx_status VX_CALLBACK vxPhaseInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0 || index == 1)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

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
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_status VX_CALLBACK vxPhaseOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, 0);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_uint32 width = 0, height = 0;
            vx_df_image format = 0;


            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            ptr->type = VX_TYPE_IMAGE;
            ptr->dim.image.format = VX_DF_IMAGE_U8;
            ptr->dim.image.width = width;
            ptr->dim.image.height = height;
            status = VX_SUCCESS;
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_param_description_t phase_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t phase_kernel = {
    VX_KERNEL_PHASE,
    "org.khronos.openvx.phase",
    vxPhaseKernel,
    phase_kernel_params, dimof(phase_kernel_params),
    vxPhaseInputValidator,
    vxPhaseOutputValidator,
    NULL,
    NULL,
};


