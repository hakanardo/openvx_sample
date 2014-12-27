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
 * \brief The Copy Object Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \defgroup group_debug_ext Debugging Extension
 */

#include <stdio.h>
#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>
#include <debug_k.h>


static vx_status VX_CALLBACK vxCopyImagePtrKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 2)
    {
        vx_scalar input = (vx_scalar)parameters[0];
        vx_image output = (vx_image)parameters[1];
        vx_uint32 width = 0, height = 0, p = 0, y = 0, len = 0;
        vx_size planes = 0;
        void *src = NULL;
        void *dst = NULL;
        vx_imagepatch_addressing_t dst_addr;
        vx_rectangle_t rect;
        vx_uint8 *srcp = NULL;

        vxAccessScalarValue(input, &src);
        srcp = (vx_uint8 *)src;

        status = VX_SUCCESS; // assume success until an error occurs.
        status |= vxQueryImage(output, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
        status |= vxQueryImage(output, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
        status |= vxQueryImage(output, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
        rect.start_x = rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;
        for (p = 0; p < planes && status == VX_SUCCESS; p++)
        {
            status = VX_SUCCESS;
            status |= vxAccessImagePatch(output, &rect, p, &dst_addr, &dst, VX_WRITE_ONLY);
            for (y = 0; y < height && status == VX_SUCCESS; y+=dst_addr.step_y)
            {
                vx_uint8 *dstp = vxFormatImagePatchAddress2d(dst, 0, y, &dst_addr);
                len = (dst_addr.stride_x * dst_addr.dim_x * dst_addr.scale_x) / VX_SCALE_UNITY;
                memcpy(dstp, srcp, len);
                srcp += len;
            }
            if (status == VX_SUCCESS) {
                status |= vxCommitImagePatch(output, &rect, p, &dst_addr, dst);
            }
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

static vx_status VX_CALLBACK vxCopyImageKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 2)
    {
        vx_image input  = (vx_image)parameters[0];
        vx_image output = (vx_image)parameters[1];
        return vxCopyImage(input, output);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxCopyArrayKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 2)
    {
        vx_array src = (vx_array)parameters[0];
        vx_array dst = (vx_array)parameters[1];
        return vxCopyArray(src, dst);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxAllPassInputValidator(vx_node node, vx_uint32 index)
{
    /* as long as it's an image, we don't care */
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK vxAllPassOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    return VX_SUCCESS;
}

static vx_status VX_CALLBACK vxCopyImageOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0); /* input image */
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(vx_image));
            if (input)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_VIRT;

                status = VX_SUCCESS;
                status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status |= vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                vxReleaseImage(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCopyArrayOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index > 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0);
        if (param)
        {
            vx_array input;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(vx_array));
            if (input)
            {
                vx_enum item_type = VX_TYPE_INVALID;
                vx_size capacity = 0ul;
                status = VX_SUCCESS;
                status |= vxQueryArray(input, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &item_type, sizeof(item_type));
                status |= vxQueryArray(input, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status |= vxSetMetaFormatAttribute(meta, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &item_type, sizeof(item_type));
                status |= vxSetMetaFormatAttribute(meta, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                vxReleaseArray(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}


/*! \brief Declares the parameter types for \ref vxCopyImageNode.
  * \ingroup group_debug_ext
  */
static vx_param_description_t copyimage_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

/*! \brief Declares the parameter types for \ref vxCopyArrayNode.
 * \ingroup group_debug_ext
 */
static vx_param_description_t copyarray_kernel_params[] = {
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
};

/*! \brief Declares the parameter types for \ref vxCopyImageFromPtrNode.
 * \ingroup group_debug_ext
 */
static vx_param_description_t copyimageptr_kernel_params[] = {
    {VX_INPUT, VX_TYPE_SIZE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t copyimageptr_kernel = {
    VX_KERNEL_COPY_IMAGE_FROM_PTR,
    "org.khronos.debug.copy_image_ptr",
    vxCopyImagePtrKernel,
    copyimageptr_kernel_params, dimof(copyimageptr_kernel_params),
    vxAllPassInputValidator,
    vxAllPassOutputValidator,
    NULL,
    NULL,
};

vx_kernel_description_t copyimage_kernel = {
    VX_KERNEL_DEBUG_COPY_IMAGE,
    "org.khronos.debug.copy_image",
    vxCopyImageKernel,
    copyimage_kernel_params, dimof(copyimage_kernel_params),
    vxAllPassInputValidator,
    vxCopyImageOutputValidator,
    NULL,
    NULL,
};

vx_kernel_description_t copyarray_kernel = {
    VX_KERNEL_DEBUG_COPY_ARRAY,
    "org.khronos.debug.copy_array",
    vxCopyArrayKernel,
    copyarray_kernel_params, dimof(copyarray_kernel_params),
    vxAllPassInputValidator,
    vxCopyArrayOutputValidator,
    NULL,
    NULL,
};
