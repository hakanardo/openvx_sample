/*
 * Copyright (c) 2013-2014 The Khronos Group Inc.
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
 * \brief The Add and Subtract Kernels.
 * \author Hans-Peter Nilsson <hp@axis.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>


static vx_status VX_CALLBACK vxAddSubtractInputValidator(vx_node node, vx_uint32 index)
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
            if (format == VX_DF_IMAGE_U8 || format == VX_DF_IMAGE_S16)
                status = VX_SUCCESS;
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 1)
    {
        vx_image images[2];
        vx_parameter param[2] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 1),
        };
        vxQueryParameter(param[0], VX_PARAMETER_ATTRIBUTE_REF, &images[0], sizeof(images[0]));
        vxQueryParameter(param[1], VX_PARAMETER_ATTRIBUTE_REF, &images[1], sizeof(images[1]));
        if (images[0] && images[1])
        {
            vx_uint32 width[2], height[2];
            vx_df_image format1;

            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width[0], sizeof(width[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_WIDTH, &width[1], sizeof(width[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[0], sizeof(height[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[1], sizeof(height[1]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_FORMAT, &format1, sizeof(format1));
            if (width[0] == width[1] && height[0] == height[1] &&
                (format1 == VX_DF_IMAGE_U8 || format1 == VX_DF_IMAGE_S16))
                status = VX_SUCCESS;
            vxReleaseImage(&images[0]);
            vxReleaseImage(&images[1]);
        }
        vxReleaseParameter(&param[0]);
        vxReleaseParameter(&param[1]);
    }
    else if (index == 2)        /* overflow_policy: truncate or saturate. */
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
                    vx_enum overflow_policy = 0;
                    vxAccessScalarValue(scalar, &overflow_policy);
                    if ((overflow_policy == VX_CONVERT_POLICY_WRAP) ||
                        (overflow_policy == VX_CONVERT_POLICY_SATURATE))
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

static vx_status VX_CALLBACK vxAddSubtractOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        /*
         * We need to look at both input images, but only for the format:
         * if either is S16 or the output type is not U8, then it's S16.
         * The geometry of the output image is copied from the first parameter:
         * the input images are known to match from input parameters validation.
         */
        vx_parameter param[] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 1),
            vxGetParameterByIndex(node, index),
        };
        if ((param[0]) && (param[1]) && (param[2]))
        {
            vx_image images[3];
            vxQueryParameter(param[0], VX_PARAMETER_ATTRIBUTE_REF, &images[0], sizeof(images[0]));
            vxQueryParameter(param[1], VX_PARAMETER_ATTRIBUTE_REF, &images[1], sizeof(images[1]));
            vxQueryParameter(param[2], VX_PARAMETER_ATTRIBUTE_REF, &images[2], sizeof(images[2]));
            if (images[0] && images[1] && images[2])
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image informat[2] = {VX_DF_IMAGE_VIRT, VX_DF_IMAGE_VIRT};
                vx_df_image outformat = VX_DF_IMAGE_VIRT;

                /*
                 * When passing on the geometry to the output image, we only look at
                 * image 0, as both input images are verified to match, at input
                 * validation.
                 */
                vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_FORMAT, &informat[0], sizeof(informat[0]));
                vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_FORMAT, &informat[1], sizeof(informat[1]));
                vxQueryImage(images[2], VX_IMAGE_ATTRIBUTE_FORMAT, &outformat, sizeof(outformat));

                if (informat[0] == VX_DF_IMAGE_U8 && informat[1] == VX_DF_IMAGE_U8 && outformat == VX_DF_IMAGE_U8)
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    outformat = VX_DF_IMAGE_S16;
                    status = VX_SUCCESS;
                }
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = outformat;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                vxReleaseImage(&images[0]);
                vxReleaseImage(&images[1]);
                vxReleaseImage(&images[2]);
            }
            vxReleaseParameter(&param[0]);
            vxReleaseParameter(&param[1]);
            vxReleaseParameter(&param[2]);
        }
    }

    return status;
}


static vx_param_description_t add_subtract_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

/* There's already a "vxAddKernel"; we have to use a slightly different name. */
static vx_status VX_CALLBACK vxAdditionKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 4)
    {
        vx_image in0 = (vx_image)parameters[0];
        vx_image in1 = (vx_image)parameters[1];
        vx_scalar policy_param = (vx_scalar)parameters[2];
        vx_image output = (vx_image)parameters[3];
        return vxAddition(in0, in1, policy_param, output);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

vx_kernel_description_t add_kernel = {
    VX_KERNEL_ADD,
    "org.khronos.openvx.add",
    vxAdditionKernel,
    add_subtract_kernel_params, dimof(add_subtract_kernel_params),
    vxAddSubtractInputValidator,
    vxAddSubtractOutputValidator,
    NULL,
    NULL,
};

static vx_status VX_CALLBACK vxSubtractionKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 4)
    {
        vx_image in0 = (vx_image)parameters[0];
        vx_image in1 = (vx_image)parameters[1];
        vx_scalar policy_param = (vx_scalar)parameters[2];
        vx_image output = (vx_image)parameters[3];
        return vxSubtraction(in0, in1, policy_param, output);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

vx_kernel_description_t subtract_kernel = {
    VX_KERNEL_SUBTRACT,
    "org.khronos.openvx.subtract",
    vxSubtractionKernel,
    add_subtract_kernel_params, dimof(add_subtract_kernel_params),
    vxAddSubtractInputValidator,
    vxAddSubtractOutputValidator,
    NULL,
    NULL,
};

