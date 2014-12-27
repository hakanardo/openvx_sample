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
 * \brief The Fill Object Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \defgroup group_debug_ext Debugging Extension
 */

#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>

static vx_status VX_CALLBACK vxFillImageKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    if (num == 2)
    {
        vx_scalar fill = (vx_scalar)parameters[0];
        vx_image image = (vx_image)parameters[1];
        vx_uint8 value[4] = {0xDE, 0xAD, 0xBE, 0xEF};
        vx_uint32 count = 0u, x = 0u, y = 0u, p = 0u;
        vx_size planes = 0u;
        vx_int32 i = 0;

        vxAccessScalarValue(fill, value);
        vxQueryImage(image, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
        for (p = 0u; p < planes; p++)
        {
            void *ptr = NULL;
            vx_imagepatch_addressing_t addr;
            vx_rectangle_t rect;
            status = vxGetValidRegionImage(image, &rect);
            status |= vxAccessImagePatch(image, &rect, p, &addr, &ptr, VX_WRITE_ONLY);
            if ((status == VX_SUCCESS) && (ptr))
            {
                for (y = 0u; y < addr.dim_y; y+=addr.step_y)
                {
                    for (x = 0u; x < addr.dim_x; x+=addr.step_x)
                    {
                        vx_uint8 *pixel = vxFormatImagePatchAddress2d(ptr, x, y, &addr);
                        for (i = 0; i < addr.stride_x; i++)
                        {
                            pixel[i] = value[i];
                            count++;
                        }
                    }
                }
                status = vxCommitImagePatch(image, &rect, p, &addr, ptr);
                if (status != VX_SUCCESS)
                {
                    vxAddLogEntry((vx_reference)node, VX_FAILURE, "Failed to set image patch for "VX_FMT_REF"\n", image);
                }
            }
            else
            {
                vxAddLogEntry((vx_reference)node, VX_FAILURE, "Failed to get image patch for "VX_FMT_REF"\n", image);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxFillImageInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar scalar;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum stype = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
                if (stype == VX_TYPE_UINT32)
                {
                    status = VX_SUCCESS;
                }
                vxReleaseScalar(&scalar);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxFillImageOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_FAILURE;
    if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image image = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &image, sizeof(image));
            if (image)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_VIRT;

                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxReleaseImage(&image);

                status = VX_SUCCESS;
            }
            else
            {
                status = VX_ERROR_INVALID_REFERENCE;
            }
            vxReleaseParameter(&param);
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    return status;
}

static vx_param_description_t fill_image_params[] = {
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t fillimage_kernel = {
    VX_KERNEL_FILL_IMAGE,
    "org.khronos.debug.fill_image",
    vxFillImageKernel,
    fill_image_params, dimof(fill_image_params),
    vxFillImageInputValidator,
    vxFillImageOutputValidator,
    NULL, NULL,
};
