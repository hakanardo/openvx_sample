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


static vx_status VX_CALLBACK vxFast9CornersKernel(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 5)
    {
        vx_border_mode_t bordermode;
        vx_image src = (vx_image)parameters[0];
        vx_scalar sens = (vx_scalar)parameters[1];
        vx_scalar nonm = (vx_scalar)parameters[2];
        vx_array points = (vx_array)parameters[3];
        vx_scalar num_corners = (vx_scalar)parameters[4];
        status = vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));
        if (status == VX_SUCCESS)
        {
            status = vxFast9Corners(src, sens, nonm, points, num_corners, &bordermode);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxFast9InputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image input = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if ((status == VX_SUCCESS) && (input))
            {
                vx_df_image format = 0;
                status = vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if ((status == VX_SUCCESS) && (format == VX_DF_IMAGE_U8))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar sens = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
            if ((status == VX_SUCCESS) && (sens))
            {
                vx_enum type = VX_TYPE_INVALID;
                vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32)
                {
                    vx_float32 k = 0.0f;
                    status = vxAccessScalarValue(sens, &k);
                    if ((status == VX_SUCCESS) && (k > 0) && (k < 256))
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
                vxReleaseScalar(&sens);
            }
            vxReleaseParameter(&param);
        }
    }
    if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar s_nonmax = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &s_nonmax, sizeof(s_nonmax));
            if ((status == VX_SUCCESS) && (s_nonmax))
            {
                vx_enum type = VX_TYPE_INVALID;
                vxQueryScalar(s_nonmax, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_BOOL)
                {
                    vx_bool nonmax = vx_false_e;
                    status = vxAccessScalarValue(s_nonmax, &nonmax);
                    if ((status == VX_SUCCESS) && ((nonmax == vx_false_e) ||
                                                   (nonmax == vx_true_e)))
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
                vxReleaseScalar(&s_nonmax);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxFast9OutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        ptr->type = VX_TYPE_ARRAY;
        ptr->dim.array.item_type = VX_TYPE_KEYPOINT;
        ptr->dim.array.capacity = 0; /* no defined capacity requirement */
        status = VX_SUCCESS;
    }
    else if (index == 4)
    {
        ptr->dim.scalar.type = VX_TYPE_UINT32;
        status = VX_SUCCESS;
    }
    return status;
}

static vx_param_description_t fast9_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

vx_kernel_description_t fast9_kernel = {
    VX_KERNEL_FAST_CORNERS,
    "org.khronos.openvx.fast_corners",
    vxFast9CornersKernel,
    fast9_kernel_params, dimof(fast9_kernel_params),
    vxFast9InputValidator,
    vxFast9OutputValidator,
};
