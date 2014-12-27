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
 * \brief The Filter Kernel (Extras)
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>

static vx_status VX_CALLBACK vxImageListerKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 3)
    {
        vx_image src = (vx_image)parameters[0];
        vx_array arr = (vx_array)parameters[1];
        vx_scalar s_num_points = (vx_scalar)parameters[2];
        vx_uint32 y, x;
        void *src_base = NULL;
        vx_imagepatch_addressing_t src_addr;
        vx_rectangle_t rect;
        vx_df_image format;
        vx_uint32 num_corners = 0;
        vx_size dst_capacity = 0;

        status = vxGetValidRegionImage(src, &rect);
        /* remove any pre-existing points */
        status |= vxTruncateArray(arr, 0);
        status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
        status |= vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
        status |= vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_CAPACITY, &dst_capacity, sizeof(dst_capacity));
        for (y = 0; y < src_addr.dim_y; y++)
        {
            for (x = 0; x < src_addr.dim_x; x++)
            {
                void *ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                if (ptr)
                {
                    vx_bool set = vx_false_e;
                    vx_float32 strength = 0.0f;
                    if (format == VX_DF_IMAGE_U8)
                    {
                        vx_uint8 pixel = *(vx_uint8 *)ptr;
                        strength = (vx_float32)pixel;
                        set = vx_true_e;
                    }
                    else if (format == VX_DF_IMAGE_S16)
                    {
                        vx_int16 pixel = *(vx_int16 *)ptr;
                        strength = (vx_float32)pixel;
                        set = vx_true_e;
                    }
                    else if (format == VX_DF_IMAGE_S32)
                    {
                        vx_int32 pixel = *(vx_int32 *)ptr;
                        strength = (vx_float32)pixel;
                        set = vx_true_e;
                    }
                    else if (format == VX_DF_IMAGE_F32)
                    {
                        vx_float32 pixel = *(vx_float32 *)ptr;
                        strength = pixel;
                        set = vx_true_e;
                    }
                    if ((set == vx_true_e) && (strength > 0.0f))
                    {
                        if (num_corners < dst_capacity)
                        {
                            vx_keypoint_t keypoint;
                            keypoint.x = rect.start_x + x;
                            keypoint.y = rect.start_y + y;
                            keypoint.strength = strength;
                            keypoint.tracking_status = 1;
                            keypoint.error = 0;
                            status |= vxAddArrayItems(arr, 1, &keypoint, 0);
                            //printf("pixel(%d,%d) = %lf (status = %d)\n",x,y,strength, status);
                        }
                        num_corners++;
                    }
                }
            }
        }
        if (s_num_points)
            status |= vxCommitScalarValue(s_num_points, &num_corners);
        status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    }
    return status;
}

static vx_status VX_CALLBACK vxListerInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_df_image format = 0;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if ((format == VX_DF_IMAGE_U8) ||
                    (format == VX_DF_IMAGE_S16) ||
                    (format == VX_DF_IMAGE_S32) ||
                    (format == VX_DF_IMAGE_F32))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxListerOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_size capacity = 0ul; /* no defined requirement */
        vx_enum type = VX_TYPE_KEYPOINT;

        status = VX_SUCCESS;
        status |= vxSetMetaFormatAttribute(meta, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
        status |= vxSetMetaFormatAttribute(meta, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &type, sizeof(type));
    }
    else if (index == 2)
    {
        vx_enum type = VX_TYPE_UINT32;
        status = VX_SUCCESS;
        status |= vxSetMetaFormatAttribute(meta, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
    }
    return status;
}

static vx_param_description_t lister_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

vx_kernel_description_t lister_kernel = {
    VX_KERNEL_EXTRAS_IMAGE_LISTER,
    "org.khronos.extras.image_to_list",
    vxImageListerKernel,
    lister_kernel_params, dimof(lister_kernel_params),
    vxListerInputValidator,
    vxListerOutputValidator,
    NULL,
    NULL,
};
