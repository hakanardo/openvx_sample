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
 * \brief The Color Convert Kernels
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>


static vx_status VX_CALLBACK vxColorConvertKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 2)
    {
        vx_image src = (vx_image)parameters[0];
        vx_image dst = (vx_image)parameters[1];

        return vxConvertColor(src, dst);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxColorConvertInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_SUCCESS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0);
        if (param)
        {
            vx_image image = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));
            if (image)
            {
                vx_df_image format = 0;
                vx_uint32 width = 0, height = 0;

                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                // check to make sure the input format is supported.
                switch (format)
                {
                    case VX_DF_IMAGE_RGB:  /* 8:8:8 interleaved */
                    case VX_DF_IMAGE_RGBX: /* 8:8:8:8 interleaved */
                    case VX_DF_IMAGE_NV12: /* 4:2:0 co-planar*/
                    case VX_DF_IMAGE_NV21: /* 4:2:0 co-planar*/
                    case VX_DF_IMAGE_IYUV: /* 4:2:0 planar */
                        if (height & 1)
                        {
                            status = VX_ERROR_INVALID_DIMENSION;
                            break;
                        }
                        /* no break */
                    case VX_DF_IMAGE_YUYV: /* 4:2:2 interleaved */
                    case VX_DF_IMAGE_UYVY: /* 4:2:2 interleaved */
                        if (width & 1)
                        {
                            status = VX_ERROR_INVALID_DIMENSION;
                        }
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
            vxReleaseParameter(&param);
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

static vx_df_image color_combos[][2] = {
        /* {src, dst} */
        {VX_DF_IMAGE_RGB, VX_DF_IMAGE_RGBX},
        {VX_DF_IMAGE_RGB, VX_DF_IMAGE_NV12},
        {VX_DF_IMAGE_RGB, VX_DF_IMAGE_YUV4},
        {VX_DF_IMAGE_RGB, VX_DF_IMAGE_IYUV},
        {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_RGB},
        {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_NV12},
        {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_YUV4},
        {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_IYUV},
        {VX_DF_IMAGE_NV12,VX_DF_IMAGE_RGB},
        {VX_DF_IMAGE_NV12,VX_DF_IMAGE_RGBX},
        {VX_DF_IMAGE_NV12,VX_DF_IMAGE_NV21},
        {VX_DF_IMAGE_NV12,VX_DF_IMAGE_YUV4},
        {VX_DF_IMAGE_NV12,VX_DF_IMAGE_IYUV},
        {VX_DF_IMAGE_NV21,VX_DF_IMAGE_RGB},
        {VX_DF_IMAGE_NV21,VX_DF_IMAGE_RGBX},
        {VX_DF_IMAGE_NV21,VX_DF_IMAGE_NV12},
        {VX_DF_IMAGE_NV21,VX_DF_IMAGE_YUV4},
        {VX_DF_IMAGE_NV21,VX_DF_IMAGE_IYUV},
        {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_RGB},
        {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_RGBX},
        {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_NV12},
        {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_YUV4},
        {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_IYUV},
        {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_RGB},
        {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_RGBX},
        {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_NV12},
        {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_YUV4},
        {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_IYUV},
        {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_RGB},
        {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_RGBX},
        {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_NV12},
        {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_YUV4},
};

static vx_status VX_CALLBACK vxColorConvertOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_parameter param0 = vxGetParameterByIndex(node, 0);
        vx_parameter param1 = vxGetParameterByIndex(node, 1);
        if (param0 && param1)
        {
            vx_image output = 0, input = 0;
            vxQueryParameter(param0, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            vxQueryParameter(param1, VX_PARAMETER_ATTRIBUTE_REF, &output, sizeof(output));
            if (input && output)
            {
                vx_df_image src = VX_DF_IMAGE_VIRT;
                vx_df_image dst = VX_DF_IMAGE_VIRT;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &src, sizeof(src));
                vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &dst, sizeof(dst));
                if (dst != VX_DF_IMAGE_VIRT) /* can't be a unspecified format */
                {
                    vx_uint32 i = 0;
                    for (i = 0; i < dimof(color_combos); i++)
                    {
                        if ((color_combos[i][0] == src) &&
                            (color_combos[i][1] == dst))
                        {
                            ptr->type = VX_TYPE_IMAGE;
                            ptr->dim.image.format = dst;
                            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &ptr->dim.image.width, sizeof(ptr->dim.image.width));
                            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &ptr->dim.image.height, sizeof(ptr->dim.image.height));
                            status = VX_SUCCESS;
                            break;
                        }
                    }
                }
                vxReleaseImage(&input);
                vxReleaseImage(&output);
            }
            vxReleaseParameter(&param0);
            vxReleaseParameter(&param1);
        }
    }
    VX_PRINT(VX_ZONE_API, "%s:%u returned %d\n", __FUNCTION__, index, status);
    return status;
}


/*! \brief Declares the parameter types for \ref vxuColorConvert.
 * \ingroup group_implementation
 */
static vx_param_description_t color_convert_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

/*! \brief The exported kernel table entry */
vx_kernel_description_t colorconvert_kernel = {
    VX_KERNEL_COLOR_CONVERT,
    "org.khronos.openvx.color_convert",
    vxColorConvertKernel,
    color_convert_kernel_params, dimof(color_convert_kernel_params),
    vxColorConvertInputValidator,
    vxColorConvertOutputValidator,
    NULL,
    NULL,
};

