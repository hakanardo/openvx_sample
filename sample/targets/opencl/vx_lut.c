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
 * \brief The TableLookup Kernel.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_interface.h>
#include <math.h>

static vx_status vxTableLookupInputValidator(vx_node node, vx_uint32 index)
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
            {
                status = VX_SUCCESS;
            }
        }
        vxReleaseParameter(&param);
    }
    else if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        vx_lut lut = 0;
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &lut, sizeof(lut));
        if (lut)
        {
            vx_enum type = 0;
            vxQueryLUT(lut, VX_LUT_ATTRIBUTE_TYPE, &type, sizeof(type));
            if (type == VX_TYPE_UINT8 || type == VX_TYPE_INT16)
            {
                status = VX_SUCCESS;
            }
        }
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_status vxTableLookupOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_image src = 0;
        vx_parameter src_param = vxGetParameterByIndex(node, 0);

        vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
        if (src)
        {
            vx_uint32 width = 0, height = 0;
            vx_df_image format;

            vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(height));
            vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
            vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            /* output is same type and dimensions */
            ptr->type = VX_TYPE_IMAGE;
            ptr->dim.image.format = format;
            ptr->dim.image.width = width;
            ptr->dim.image.height = height;
            status = VX_SUCCESS;
        }
        vxReleaseParameter(&src_param);
    }
    return status;
}

static vx_param_description_t lut_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_LUT,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_cl_kernel_description_t lut_clkernel = {
    {
        VX_KERNEL_TABLE_LOOKUP,
        "org.khronos.openvx.table_lookup",
        NULL,
        lut_kernel_params, dimof(lut_kernel_params),
        vxTableLookupInputValidator,
        vxTableLookupOutputValidator,
        NULL,
        NULL,
    },
    VX_CL_SOURCE_DIR""FILE_JOINER"vx_lut.cl",
    "vx_lut",
    INIT_PROGRAMS,
    INIT_KERNELS,
    INIT_NUMKERNELS,
    INIT_RETURNS,
    NULL,
};


