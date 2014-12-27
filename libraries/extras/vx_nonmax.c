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
 * \brief The Non-Maxima Suppression Kernel (Extras)
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>
#include <math.h>
#include <extras_k.h>


static vx_status VX_CALLBACK vxEuclideanNonMaxSuppressionKernel(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    if (num == 4)
    {
        vx_image  src = (vx_image) parameters[0];
        vx_scalar thr = (vx_scalar)parameters[1];
        vx_scalar rad = (vx_scalar)parameters[2];
        vx_image  dst = (vx_image) parameters[3];
        return vxEuclideanNonMaxSuppression(src, thr, rad, dst);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxNonMaxSuppressionKernel(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    if (num == 3)
    {
        vx_image i_mag  = (vx_image)parameters[0];
        vx_image i_ang  = (vx_image)parameters[1];
        vx_image i_edge = (vx_image)parameters[2];
        vx_border_mode_t borders;
        vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
        return vxNonMaxSuppression(i_mag, i_ang, i_edge, &borders);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxEuclideanNonMaxInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0) /* image */
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image img = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img));
            if (img)
            {
                vx_df_image format = VX_DF_IMAGE_VIRT;
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if ((format == VX_DF_IMAGE_S32) || (format == VX_DF_IMAGE_F32))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&img);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 1) /* strength_thresh */
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
                if (stype == VX_TYPE_FLOAT32)
                {
                    status = VX_SUCCESS;
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
    else if (index == 2) /* min_distance */
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
                if (stype == VX_TYPE_FLOAT32)
                {
                    vx_float32 radius = 0;
                    vxAccessScalarValue(scalar, &radius);
                    if ((1.0 < radius) && (radius <= 5.0))
                    {
                        status = VX_SUCCESS;
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

static vx_status VX_CALLBACK vxNonMaxSuppressionInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0) /* magnitude */
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image img = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img));
            if (img)
            {
                vx_df_image format = VX_DF_IMAGE_VIRT;
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U8 || format == VX_DF_IMAGE_S16 || format == VX_DF_IMAGE_U16)
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&img);
            }
            vxReleaseParameter(&param);
        }
    }
    if (index == 1)
    {
        vx_parameter params[] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 1),
        };
        if (params[0] && params[1])
        {
            vx_image images[] = {0, 0};

            vxQueryParameter(params[0], VX_PARAMETER_ATTRIBUTE_REF, &images[0], sizeof(images[0]));
            vxQueryParameter(params[1], VX_PARAMETER_ATTRIBUTE_REF, &images[1], sizeof(images[1]));
            if (images[0] && images[1])
            {
                vx_uint32 width[2], height[2];
                vx_df_image format = VX_DF_IMAGE_VIRT;
                vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width[0], sizeof(width[0]));
                vxQueryImage(images[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[0], sizeof(height[0]));
                vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_WIDTH, &width[1], sizeof(width[1]));
                vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[1], sizeof(height[1]));
                vxQueryImage(images[1], VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if ((format == VX_DF_IMAGE_U8) &&
                    (width[0] == width[1]) &&
                    (height[0] == height[1]))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&images[0]);
                vxReleaseImage(&images[1]);
            }
            vxReleaseParameter(&params[0]);
            vxReleaseParameter(&params[1]);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxNonMaxSuppressionOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0);
        if (param)
        {
            vx_image img = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img));
            if (img)
            {
                vx_df_image format = VX_DF_IMAGE_VIRT;
                vx_uint32 width, height;

                status = VX_SUCCESS;
                status |= vxQueryImage(img, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(img, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxReleaseImage(&img);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxEuclideanNonMaxOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0);
        if (param)
        {
            vx_image img = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &img, sizeof(img));
            if (img)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_VIRT; /* takes the format of the input image */

                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxQueryImage(img, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxReleaseImage(&img);

                status = VX_SUCCESS;
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_param_description_t nonmaxsuppression_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

static vx_param_description_t euclidean_non_max_suppression_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, /* strength_thresh */
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, /* min_distance */
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t nonmax_kernel = {
    VX_KERNEL_EXTRAS_NONMAXSUPPRESSION,
    "org.khronos.extra.nonmaximasuppression",
    vxNonMaxSuppressionKernel,
    nonmaxsuppression_params, dimof(nonmaxsuppression_params),
    vxNonMaxSuppressionInputValidator,
    vxNonMaxSuppressionOutputValidator,
    NULL,
    NULL,
};

vx_kernel_description_t euclidian_nonmax_kernel = {
    VX_KERNEL_EXTRAS_EUCLIDEAN_NONMAXSUPPRESSION,
    "org.khronos.extra.euclidean_nonmaxsuppression",
    vxEuclideanNonMaxSuppressionKernel,
    euclidean_non_max_suppression_params, dimof(euclidean_non_max_suppression_params),
    vxEuclideanNonMaxInputValidator,
    vxEuclideanNonMaxOutputValidator,
};
