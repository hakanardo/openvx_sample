
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
 * \brief The Check Object Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \defgroup group_debug_ext Debugging Extension
 */

#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>

typedef union _packed_value_u {
    vx_uint8  bytes[8];
    vx_uint16  word[4];
    vx_uint32 dword[2];
    vx_uint64 qword[1];
} packed_value_u;

static vx_status VX_CALLBACK vxCheckImageKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    if (num == 3)
    {
        vx_image image = (vx_image)parameters[0];
        vx_scalar fill = (vx_scalar)parameters[1];
        vx_scalar errs = (vx_scalar)parameters[2];
        packed_value_u value;
        vx_uint32 count = 0u, errors = 0u;
        vx_size planes = 0u;
        vx_uint32 x = 0u, y = 0u, p = 0u;
        vx_int32 i = 0;
        vx_imagepatch_addressing_t addr;
        vx_rectangle_t rect;

        value.dword[0] = 0xDEADBEEF;
        vxAccessScalarValue(fill, &value.dword[0]);
        vxQueryImage(image, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
        vxGetValidRegionImage(image, &rect);
        for (p = 0u; (p < planes); p++)
        {
            void *ptr = NULL;
            status = vxAccessImagePatch(image, &rect, p, &addr, &ptr, VX_READ_ONLY);
            if ((status == VX_SUCCESS) && (ptr))
            {
                for (y = 0; y < addr.dim_y; y+=addr.step_y)
                {
                    for (x = 0; x < addr.dim_x; x+=addr.step_x)
                    {
                        vx_uint8 *pixel = vxFormatImagePatchAddress2d(ptr, x, y, &addr);
                        for (i = 0; i < addr.stride_x; i++)
                        {
                            count++;
                            if (pixel[i] != value.bytes[i])
                            {
                                errors++;
                            }
                        }
                    }
                }
                if (errors > 0)
                {
                    vxAddLogEntry((vx_reference)node, VX_FAILURE, "Checked %p of %u sub-pixels with 0x%08x with %u errors\n", ptr, count, value.dword, errors);
                }
                vxCommitScalarValue(errs, &errors);
                status = vxCommitImagePatch(image, NULL, p, &addr, ptr);
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
        if (errors > 0)
        {
            status = VX_FAILURE;
        }
    }
    return status;
}


static vx_status VX_CALLBACK vxCheckArrayKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    if (num == 3)
    {
        vx_array arr = (vx_array)parameters[0];
        vx_scalar fill = (vx_scalar)parameters[1];
        vx_scalar errs = (vx_scalar)parameters[2];
        packed_value_u value;
        vx_size num_items = 0ul, item_size = 0ul, stride = 0ul;
        vx_uint32 errors = 0;
        void *ptr = NULL;
        vx_size i = 0, j = 0;

        value.dword[0] = 0xDEADBEEF;
        vxAccessScalarValue(fill, &value.dword[0]);
        vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_NUMITEMS, &num_items, sizeof(num_items));
        vxQueryArray(arr, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &item_size, sizeof(item_size));
        status = vxAccessArrayRange(arr, 0, num_items, &stride, &ptr, VX_READ_ONLY);
        if (status == VX_SUCCESS)
        {
            for (i = 0; i < num_items; ++i)
            {
                vx_uint8 *item_ptr = vxFormatArrayPointer(ptr, i, stride);
                for (j = 0; j < item_size; ++j)
                {
                    if (item_ptr[j] != value.bytes[j])
                    {
                        errors++;
                    }
                }
            }
            vxCommitScalarValue(errs, &errors);
            if (errors > 0)
            {
                vxAddLogEntry((vx_reference)node, VX_FAILURE, "Check array %p of "VX_FMT_SIZE" items with 0x%02x, found %u errors\n", ptr, num_items, value, errors);
            }
            status = vxCommitArrayRange(arr, 0, num_items, ptr);
            if (status != VX_SUCCESS)
            {
                vxAddLogEntry((vx_reference)node, VX_FAILURE, "Failed to set array range for "VX_FMT_REF"\n", arr);
            }
        }
        else
        {
            vxAddLogEntry((vx_reference)node, VX_FAILURE, "Failed to get array range for "VX_FMT_REF"\n", arr);
        }
        if (errors > 0)
        {
            status = VX_FAILURE;
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCheckImageInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        status = VX_SUCCESS;
    }
    else if (index == 1)
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
                if (stype == VX_TYPE_UINT32)
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
    return status;
}

static vx_status VX_CALLBACK vxCheckArrayInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        status = VX_SUCCESS;
    }
    else if (index == 1)
    {
        vx_parameter input = vxGetParameterByIndex(node, 0);
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (input && param)
        {
            vx_array array = 0;
            vx_scalar scalar = 0;

            vxQueryParameter(input, VX_PARAMETER_ATTRIBUTE_REF, &array, sizeof(array));
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));

            if (array && scalar)
            {
                vx_enum item_type = 0;
                vx_enum stype = 0;

                vxQueryArray(array, VX_ARRAY_ATTRIBUTE_ITEMTYPE, &item_type, sizeof(item_type));
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
                if (stype == item_type)
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    status = VX_ERROR_INVALID_TYPE;
                }
                vxReleaseArray(&array);
                vxReleaseScalar(&scalar);
            }
            vxReleaseParameter(&input);
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCheckOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_enum stype =  VX_TYPE_UINT32;
        vxSetMetaFormatAttribute(meta, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
        status = VX_SUCCESS;
    }
    return status;
}

static vx_param_description_t check_image_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
static vx_param_description_t check_array_params[] = {
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t checkimage_kernel = {
    VX_KERNEL_CHECK_IMAGE,
    "org.khronos.debug.check_image",
    vxCheckImageKernel,
    check_image_params, dimof(check_image_params),
    vxCheckImageInputValidator,
    vxCheckOutputValidator,
    NULL, NULL,
};

vx_kernel_description_t checkarray_kernel = {
    VX_KERNEL_CHECK_ARRAY,
    "org.khronos.debug.check_array",
    vxCheckArrayKernel,
    check_array_params, dimof(check_array_params),
    vxCheckArrayInputValidator,
    vxCheckOutputValidator,
    NULL, NULL,
};
