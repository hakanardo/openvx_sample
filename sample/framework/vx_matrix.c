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

void vxDestructMatrix(vx_reference ref)
{
    vx_matrix matrix = (vx_matrix)ref;
    vxFreeMemory(matrix->base.context, &matrix->memory);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseMatrix(vx_matrix *mat)
{
    return vxReleaseReferenceInt((vx_reference *)mat, VX_TYPE_MATRIX, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_matrix VX_API_CALL vxCreateMatrix(vx_context context, vx_enum data_type, vx_size columns, vx_size rows)
{
    vx_matrix matrix = NULL;
    vx_size dim = 0ul;
    if (vxIsValidContext(context) == vx_false_e)
        return 0;

    if ((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8))
    {
        dim = sizeof(vx_uint8);
    }
    else if ((data_type == VX_TYPE_INT16) || (data_type == VX_TYPE_UINT16))
    {
        dim = sizeof(vx_uint16);
    }
    else if ((data_type == VX_TYPE_INT32) || (data_type == VX_TYPE_UINT32) || (data_type == VX_TYPE_FLOAT32))
    {
        dim = sizeof(vx_uint32);
    }
    else if ((data_type == VX_TYPE_INT64) || (data_type == VX_TYPE_UINT64) || (data_type == VX_TYPE_FLOAT64))
    {
        dim = sizeof(vx_uint64);
    }
    if (dim == 0ul)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid data type\n");
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_TYPE, "Invalid data type\n");
        return (vx_matrix)vxGetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }
    if ((columns == 0ul) || (rows == 0ul))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid dimensions to matrix\n");
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_DIMENSION, "Invalid dimensions to matrix\n");
        return (vx_matrix)vxGetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }
    matrix = (vx_matrix)vxCreateReference(context, VX_TYPE_MATRIX, VX_EXTERNAL, &context->base);
    if (matrix && matrix->base.type == VX_TYPE_MATRIX)
    {
        matrix->data_type = data_type;
        matrix->columns = columns;
        matrix->rows = rows;
        matrix->memory.ndims = 2;
        matrix->memory.nptrs = 1;
        matrix->memory.dims[0][0] = (vx_int32)dim;
        matrix->memory.dims[0][1] = (vx_int32)(columns*rows);
    }
    return (vx_matrix)matrix;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryMatrix(vx_matrix matrix, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_MATRIX_ATTRIBUTE_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
            {
                *(vx_enum *)ptr = matrix->data_type;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_MATRIX_ATTRIBUTE_ROWS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = matrix->rows;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_MATRIX_ATTRIBUTE_COLUMNS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = matrix->columns;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_MATRIX_ATTRIBUTE_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = matrix->columns * matrix->rows * matrix->memory.dims[0][0];
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

VX_API_ENTRY vx_status VX_API_CALL vxAccessMatrix(vx_matrix matrix, void *array)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_true_e)
    {
        if (vxAllocateMemory(matrix->base.context, &matrix->memory) == vx_true_e)
        {
            vxSemWait(&matrix->base.lock);
            if (array)
            {
                vx_size size = matrix->memory.strides[0][1] *
                               matrix->memory.dims[0][1];
                memcpy(array, matrix->memory.ptrs[0], size);
            }
            vxSemPost(&matrix->base.lock);
            vxReadFromReference(&matrix->base);
            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate matrix\n");
            status = VX_ERROR_NO_MEMORY;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference for matrix\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitMatrix(vx_matrix matrix, void *array)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidSpecificReference(&matrix->base, VX_TYPE_MATRIX) == vx_true_e)
    {
        if (vxAllocateMemory(matrix->base.context, &matrix->memory) == vx_true_e)
        {
            vxSemWait(&matrix->base.lock);
            if (array)
            {
                vx_size size = matrix->memory.strides[0][1] *
                               matrix->memory.dims[0][1];
                memcpy(matrix->memory.ptrs[0], array, size);
            }
            vxSemPost(&matrix->base.lock);
            vxWroteToReference(&matrix->base);
            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate matrix\n");
            status = VX_ERROR_NO_MEMORY;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid reference for matrix\n");
    }
    return status;
}
