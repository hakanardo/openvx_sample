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
 * \brief The Channel Combine Kernels
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>

static vx_status VX_CALLBACK vxChannelCombineKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 5)
    {
        vx_image inputs[4] = {
            (vx_image)parameters[0],
            (vx_image)parameters[1],
            (vx_image)parameters[2],
            (vx_image)parameters[3],
        };
        vx_image output = (vx_image)parameters[4];

        return vxChannelCombine(inputs, output);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxChannelCombineInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index < 4)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image image = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));
            if (image)
            {
                vx_df_image format = 0;
                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U8)
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&image);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxChannelCombineOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 4)
    {
        vx_uint32 p, width = 0, height = 0;
        vx_uint32 uv_x_scale = 0, uv_y_scale = 0;
        vx_parameter params[] = {
                vxGetParameterByIndex(node, 0),
                vxGetParameterByIndex(node, 1),
                vxGetParameterByIndex(node, 2),
                vxGetParameterByIndex(node, 3),
                vxGetParameterByIndex(node, index)
        };
        vx_bool planes_present[4] = { vx_false_e, vx_false_e, vx_false_e, vx_false_e };
        /* check for equal plane sizes and determine plane presence */
        for (p = 0; p < index; p++)
        {
            if (params[p])
            {
                vx_image image = 0;
                vxQueryParameter(params[p], VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));
                planes_present[p] = image != 0;

                if (image)
                {
                    uint32_t w = 0, h = 0;
                    vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &w, sizeof(w));
                    vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &h, sizeof(h));
                    if (width == 0 && height == 0)
                    {
                        width = w;
                        height = h;
                    }
                    else if (uv_x_scale == 0 && uv_y_scale == 0)
                    {
                        uv_x_scale = width == w ? 1 : (width == 2*w ? 2 : 0);
                        uv_y_scale = height == h ? 1 : (height == 2*h ? 2 : 0);
                        if (uv_x_scale == 0 || uv_y_scale == 0 || uv_y_scale > uv_x_scale)
                        {
                            status = VX_ERROR_INVALID_DIMENSION;
                            vxAddLogEntry((vx_reference)image, status, "Input image channel %u does not match in dimensions!\n", p);
                            goto exit;
                        }
                    }
                    else if (width != w * uv_x_scale || height != h * uv_y_scale)
                    {
                        status = VX_ERROR_INVALID_DIMENSION;
                        vxAddLogEntry((vx_reference)image, status, "Input image channel %u does not match in dimensions!\n", p);
                        goto exit;
                    }
                    vxReleaseImage(&image);
                }
            }
        }
        if (params[index])
        {
            vx_image output = 0;
            vxQueryParameter(params[index], VX_PARAMETER_ATTRIBUTE_REF, &output, sizeof(output));
            if (output)
            {
                vx_df_image format = VX_DF_IMAGE_VIRT;
                vx_bool supported_format = vx_true_e;
                vx_bool correct_planes = planes_present[0] && planes_present[1] && planes_present[2];

                vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                switch (format)
                {
                    case VX_DF_IMAGE_RGB:
                    case VX_DF_IMAGE_YUV4:
                        correct_planes = correct_planes && uv_y_scale == 1 && uv_x_scale == 1;
                        break;
                    case VX_DF_IMAGE_RGBX:
                        correct_planes = correct_planes && planes_present[3] && uv_y_scale == 1 && uv_x_scale == 1;
                        break;
                    case VX_DF_IMAGE_YUYV:
                    case VX_DF_IMAGE_UYVY:
                        correct_planes = correct_planes && uv_y_scale == 1 && uv_x_scale == 2;
                        break;
                    case VX_DF_IMAGE_NV12:
                    case VX_DF_IMAGE_NV21:
                    case VX_DF_IMAGE_IYUV:
                        correct_planes = correct_planes && uv_y_scale == 2 && uv_x_scale == 2;
                        break;
                    default:
                        supported_format = vx_false_e;
                }
                if (supported_format)
                {
                    if (correct_planes)
                    {
                        ptr->type = VX_TYPE_IMAGE;
                        ptr->dim.image.format = format;
                        ptr->dim.image.width = width;
                        ptr->dim.image.height = height;
                        status = VX_SUCCESS;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_API, "Valid format but missing planes!\n");
                    }
                }
                vxReleaseImage(&output);
            }
        }
exit:
        for (p = 0; p < dimof(params); p++)
        {
            if (params[p])
            {
                vxReleaseParameter(&params[p]);
            }
        }
    }
    VX_PRINT(VX_ZONE_API, "%s:%u returned %d\n", __FUNCTION__, index, status);
    return status;
}


static vx_param_description_t channel_combine_kernel_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t channelcombine_kernel = {
    VX_KERNEL_CHANNEL_COMBINE,
    "org.khronos.openvx.channel_combine",
    vxChannelCombineKernel,
    channel_combine_kernel_params, dimof(channel_combine_kernel_params),
    vxChannelCombineInputValidator,
    vxChannelCombineOutputValidator,
    NULL,
    NULL,
};
