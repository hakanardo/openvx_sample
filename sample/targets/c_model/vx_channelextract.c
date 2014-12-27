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
 * \brief The Channel Extract Kernels
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>


static vx_status VX_CALLBACK vxChannelExtractKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 3)
    {
        vx_image src = (vx_image)parameters[0];
        vx_scalar channel = (vx_scalar)parameters[1];
        vx_image dst = (vx_image)parameters[2];
        return vxChannelExtract(src, channel, dst);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxChannelExtractInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    vx_parameter param = vxGetParameterByIndex(node, index);
    if (index == 0)
    {
        vx_image image = 0;
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));
        if (image)
        {
            vx_df_image format = 0;
            vx_uint32 width, height;
            vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
            vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
            // check to make sure the input format is supported.
            switch (format)
            {
                case VX_DF_IMAGE_RGB:
                case VX_DF_IMAGE_RGBX:
                case VX_DF_IMAGE_YUV4:
                    status = VX_SUCCESS;
                    break;
                /* 4:2:0 */
                case VX_DF_IMAGE_NV12:
                case VX_DF_IMAGE_NV21:
                case VX_DF_IMAGE_IYUV:
                    if (width % 2 != 0 || height % 2 != 0)
                        status = VX_ERROR_INVALID_DIMENSION;
                    else
                        status = VX_SUCCESS;
                    break;
                /* 4:2:2 */
                case VX_DF_IMAGE_UYVY:
                case VX_DF_IMAGE_YUYV:
                    if (width % 2 != 0)
                        status = VX_ERROR_INVALID_DIMENSION;
                    else
                        status = VX_SUCCESS;
                    break;
                default:
                    status = VX_ERROR_INVALID_FORMAT;
                    break;
            }
            vxReleaseImage(&image);
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else if (index == 1)
    {
        vx_scalar scalar;
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
        if (scalar)
        {
            vx_enum type = 0;
            vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
            if (type == VX_TYPE_ENUM)
            {
                vx_enum channel = 0;
                vx_parameter param0;

                vxAccessScalarValue(scalar, &channel);
                param0 = vxGetParameterByIndex(node, 0);

                if (param0)
                {
                    vx_image image = 0;
                    vxQueryParameter(param0, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));

                    if (image)
                    {
                        vx_df_image format = VX_DF_IMAGE_VIRT;
                        vx_enum max_channel;
                        vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                        max_channel = (format == VX_DF_IMAGE_RGBX ? VX_CHANNEL_3 : VX_CHANNEL_2);

                        if (VX_CHANNEL_0 <= channel && channel <= max_channel)
                        {
                            status = VX_SUCCESS;
                        }
                        else
                        {
                            status = VX_ERROR_INVALID_VALUE;
                        }

                        vxReleaseImage(&image);
                    }

                    vxReleaseParameter(&param0);
                }
            }
            else
            {
                status = VX_ERROR_INVALID_TYPE;
            }
            vxReleaseScalar(&scalar);
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    vxReleaseParameter(&param);
    return status;
}

static vx_status VX_CALLBACK vxChannelExtractOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter param0 = vxGetParameterByIndex(node, 0);
        vx_parameter param1 = vxGetParameterByIndex(node, 1);

        if ((param0) && (param1))
        {
            vx_image input = 0;
            vx_scalar chan = 0;
            vx_enum channel = 0;
            vxQueryParameter(param0, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            vxQueryParameter(param1, VX_PARAMETER_ATTRIBUTE_REF, &chan, sizeof(chan));
            vxAccessScalarValue(chan, &channel);

            if ((input) && (chan))
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_VIRT;

                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                if (channel != VX_CHANNEL_0)
                    switch (format) {
                        case VX_DF_IMAGE_IYUV:
                        case VX_DF_IMAGE_NV12:
                        case VX_DF_IMAGE_NV21:
                            width /= 2;
                            height /= 2;
                            break;
                        case VX_DF_IMAGE_YUYV:
                        case VX_DF_IMAGE_UYVY:
                            width /= 2;
                            break;
                    }

                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = VX_DF_IMAGE_U8;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                status = VX_SUCCESS;
                vxReleaseImage(&input);
                vxReleaseScalar(&chan);
            }
            vxReleaseParameter(&param0);
            vxReleaseParameter(&param1);
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    VX_PRINT(VX_ZONE_API, "%s:%u returned %d\n", __FUNCTION__, index, status);
    return status;
}


/*! \brief Declares the parameter types for \ref vxuColorConvert.
 * \ingroup group_implementation
 */
static vx_param_description_t channel_extract_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

/*! \brief The exported kernel table entry */
vx_kernel_description_t channelextract_kernel = {
    VX_KERNEL_CHANNEL_EXTRACT,
    "org.khronos.openvx.channel_extract",
    vxChannelExtractKernel,
    channel_extract_kernel_params, dimof(channel_extract_kernel_params),
    vxChannelExtractInputValidator,
    vxChannelExtractOutputValidator,
    NULL,
    NULL,
};

