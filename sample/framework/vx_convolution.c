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

#include <vx_internal.h>

void vxDestructConvolution(vx_reference ref)
{
    vx_convolution convolution = (vx_convolution)ref;
    vxFreeMemory(convolution->base.base.context, &convolution->base.memory);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseConvolution(vx_convolution *conv)
{
    return vxReleaseReferenceInt((vx_reference *)conv, VX_TYPE_CONVOLUTION, VX_EXTERNAL, NULL);
}

static VX_INLINE int isodd(size_t a)
{
    return (int)(a & 1);
}

VX_API_ENTRY vx_convolution VX_API_CALL vxCreateConvolution(vx_context context, vx_size columns, vx_size rows)
{
    vx_convolution convolution = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (isodd(columns) && columns >= 3 && isodd(rows) && rows >= 3)
        {
            convolution = (vx_convolution)vxCreateReference(context, VX_TYPE_CONVOLUTION, VX_EXTERNAL, &context->base);
            if (convolution && convolution->base.base.type == VX_TYPE_CONVOLUTION)
            {
                convolution->base.data_type = VX_TYPE_INT16;
                convolution->base.columns = columns;
                convolution->base.rows = rows;
                convolution->base.memory.ndims = 2;
                convolution->base.memory.nptrs = 1;
                convolution->base.memory.dims[0][0] = sizeof(vx_int16);
                convolution->base.memory.dims[0][1] = (vx_int32)(columns*rows);
                convolution->scale = 1;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to create convolution, invalid dimensions\n");
            vxAddLogEntry((vx_reference)context, VX_ERROR_INVALID_DIMENSION, "Invalid dimensions to convolution\n");
            convolution = (vx_convolution )vxGetErrorObject((vx_context )context, VX_ERROR_INVALID_DIMENSION);
        }
    }
    return convolution;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryConvolution(vx_convolution convolution, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&convolution->base.base, VX_TYPE_CONVOLUTION) == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_CONVOLUTION_ATTRIBUTE_ROWS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = convolution->base.rows;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_CONVOLUTION_ATTRIBUTE_COLUMNS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = convolution->base.columns;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_CONVOLUTION_ATTRIBUTE_SCALE:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32 *)ptr = convolution->scale;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_CONVOLUTION_ATTRIBUTE_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = convolution->base.columns * convolution->base.rows * sizeof(vx_int16);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}

static vx_bool vxIsPowerOfTwo(vx_uint32 a)
{
    if (a == 0)
        return vx_false_e;
    else if ((a & ((a) - 1)) == 0)
        return vx_true_e;
    else
        return vx_false_e;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetConvolutionAttribute(vx_convolution convolution, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&convolution->base.base, VX_TYPE_CONVOLUTION) == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_CONVOLUTION_ATTRIBUTE_SCALE:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                vx_uint32 scale = *(vx_uint32 *)ptr;
                if (vxIsPowerOfTwo(scale) == vx_true_e)
                {
                    VX_PRINT(VX_ZONE_INFO, "Convolution Scale assigned to %u\n", scale);
                    convolution->scale = scale;
                }
                else
                {
                    status = VX_ERROR_INVALID_VALUE;
                }
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        default:
            status = VX_ERROR_INVALID_PARAMETERS;
            break;
    }
    if (status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to set attribute on convolution! (%d)\n", status);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessConvolutionCoefficients(vx_convolution convolution, vx_int16 *array)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((vxIsValidSpecificReference(&convolution->base.base, VX_TYPE_CONVOLUTION) == vx_true_e) &&
        (vxAllocateMemory(convolution->base.base.context, &convolution->base.memory) == vx_true_e))
    {
        vxSemWait(&convolution->base.base.lock);
        if (array)
        {
            vx_size size = convolution->base.memory.strides[0][1] *
                           convolution->base.memory.dims[0][1];
            memcpy(array, convolution->base.memory.ptrs[0], size);
        }
        vxSemPost(&convolution->base.base.lock);
        vxReadFromReference(&convolution->base.base);
        vxIncrementReference(&convolution->base.base, VX_EXTERNAL);
        status = VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitConvolutionCoefficients(vx_convolution convolution, vx_int16 *array)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((vxIsValidSpecificReference(&convolution->base.base, VX_TYPE_CONVOLUTION) == vx_true_e) &&
        (vxAllocateMemory(convolution->base.base.context, &convolution->base.memory) == vx_true_e))
    {
        vxSemWait(&convolution->base.base.lock);
        if (array)
        {
            vx_size size = convolution->base.memory.strides[0][1] *
                           convolution->base.memory.dims[0][1];

            memcpy(convolution->base.memory.ptrs[0], array, size);
        }
        vxSemPost(&convolution->base.base.lock);
        vxWroteToReference(&convolution->base.base);
        vxDecrementReference(&convolution->base.base, VX_EXTERNAL);
        status = VX_SUCCESS;
    }
    return status;
}
