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
 * \brief The Accumulation Kernels
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>
#include <vx_internal.h>
#include <c_model.h>


static vx_status VX_CALLBACK vxAccumulateKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 2)
    {
        vx_image input = (vx_image)parameters[0];
        vx_image accum = (vx_image)parameters[1];
        return vxAccumulate(input, accum);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxAccumulateWeightedKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 3)
    {
        vx_image input = (vx_image)parameters[0];
        vx_scalar scalar = (vx_scalar)parameters[1];
        vx_image accum = (vx_image)parameters[2];
        return vxAccumulateWeighted(input, scalar, accum);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxAccumulateSquareKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 3)
    {
        vx_image input = (vx_image)parameters[0];
        vx_scalar scalar = (vx_scalar)parameters[1];
        vx_image accum = (vx_image)parameters[2];
        return vxAccumulateSquare(input, scalar, accum);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxAccumulateInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0 )
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_U8)
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
            vx_df_image format[2];

            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width[0], sizeof(width[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_WIDTH, &width[1], sizeof(width[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[0], sizeof(height[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[1], sizeof(height[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_FORMAT, &format[0], sizeof(format[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_FORMAT, &format[1], sizeof(format[1]));
            if (width[0] == width[1] &&
               height[0] == height[1] &&
               format[0] == VX_DF_IMAGE_U8 &&
               format[1] == VX_DF_IMAGE_S16)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&images[0]);
            vxReleaseImage(&images[1]);
        }
        vxReleaseParameter(&param[0]);
        vxReleaseParameter(&param[1]);
    }
    return status;
}


static vx_status VX_CALLBACK vxAccumulateWeightedInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0 )
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_U8)
                status = VX_SUCCESS;
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 2)
    {
        vx_image images[2];
        vx_parameter param[2] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 2),
        };
        vxQueryParameter(param[0], VX_PARAMETER_ATTRIBUTE_REF, &images[0], sizeof(images[0]));
        vxQueryParameter(param[1], VX_PARAMETER_ATTRIBUTE_REF, &images[1], sizeof(images[1]));
        if (images[0] && images[1])
        {
            vx_uint32 width[2], height[2];
            vx_df_image format[2];

            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width[0], sizeof(width[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_WIDTH, &width[1], sizeof(width[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[0], sizeof(height[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[1], sizeof(height[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_FORMAT, &format[0], sizeof(format[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_FORMAT, &format[1], sizeof(format[1]));
            if (width[0] == width[1] &&
               height[0] == height[1] &&
               format[0] == VX_DF_IMAGE_U8 &&
               format[1] == VX_DF_IMAGE_U8)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&images[0]);
            vxReleaseImage(&images[1]);
        }
        vxReleaseParameter(&param[0]);
        vxReleaseParameter(&param[1]);
    }
    else if (index == 1) /* only weighted/squared average */
    {
        vx_scalar scalar = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum type = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32)
                {
                    vx_float32 alpha = 0.0f;
                    if ((vxAccessScalarValue(scalar, &alpha) == VX_SUCCESS) &&
                        (0.0f <= alpha) && (alpha <= 1.0f))
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

static vx_status VX_CALLBACK vxAccumulateSquaredInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0 )
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_U8)
                status = VX_SUCCESS;
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 2)
    {
        vx_image images[2];
        vx_parameter param[2] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 2),
        };
        vxQueryParameter(param[0], VX_PARAMETER_ATTRIBUTE_REF, &images[0], sizeof(images[0]));
        vxQueryParameter(param[1], VX_PARAMETER_ATTRIBUTE_REF, &images[1], sizeof(images[1]));
        if (images[0] && images[1])
        {
            vx_uint32 width[2], height[2];
            vx_df_image format[2];

            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width[0], sizeof(width[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_WIDTH, &width[1], sizeof(width[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[0], sizeof(height[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[1], sizeof(height[1]));
            vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_FORMAT, &format[0], sizeof(format[0]));
            vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_FORMAT, &format[1], sizeof(format[1]));
            if (width[0] == width[1] &&
               height[0] == height[1] &&
               format[0] == VX_DF_IMAGE_U8 &&
               format[1] == VX_DF_IMAGE_S16)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&images[0]);
            vxReleaseImage(&images[1]);
        }
        vxReleaseParameter(&param[0]);
        vxReleaseParameter(&param[1]);
    }
    else if (index == 1) /* only weighted/squared average */
    {
        vx_scalar scalar = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum type = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_UINT32)
                {
                    vx_uint32 shift = 0u;
                    if ((vxAccessScalarValue(scalar, &shift) == VX_SUCCESS) &&
                        (0 <= shift) && (shift <= 15))
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

static vx_status VX_CALLBACK vxAccumulateOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    return status;
}

static vx_param_description_t accumulate_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};


static vx_param_description_t accumulate_scaled_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};


vx_kernel_description_t accumulate_kernel = {
    VX_KERNEL_ACCUMULATE,
    "org.khronos.openvx.accumulate",
    vxAccumulateKernel,
    accumulate_kernel_params, dimof(accumulate_kernel_params),
    vxAccumulateInputValidator,
    vxAccumulateOutputValidator,
    NULL,
    NULL,
};

vx_kernel_description_t accumulate_weighted_kernel = {
    VX_KERNEL_ACCUMULATE_WEIGHTED,
    "org.khronos.openvx.accumulate_weighted",
    vxAccumulateWeightedKernel,
    accumulate_scaled_kernel_params, dimof(accumulate_scaled_kernel_params),
    vxAccumulateWeightedInputValidator,
    vxAccumulateOutputValidator,
    NULL,
    NULL,
};

vx_kernel_description_t accumulate_square_kernel = {
    VX_KERNEL_ACCUMULATE_SQUARE,
    "org.khronos.openvx.accumulate_square",
    vxAccumulateSquareKernel,
    accumulate_scaled_kernel_params, dimof(accumulate_scaled_kernel_params),
    vxAccumulateSquaredInputValidator,
    vxAccumulateOutputValidator,
    NULL,
    NULL,
};

