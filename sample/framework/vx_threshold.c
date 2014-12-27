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

static vx_bool vxIsValidThresholdType(vx_enum thresh_type)
{
    vx_bool ret = vx_false_e;
    if ((thresh_type == VX_THRESHOLD_TYPE_BINARY) ||
        (thresh_type == VX_THRESHOLD_TYPE_RANGE))
    {
        ret = vx_true_e;
    }
    return ret;
}

static vx_bool vxIsValidThresholdDataType(vx_enum data_type)
{
    vx_bool ret = vx_false_e;
    if (data_type == VX_TYPE_UINT8)
    {
        ret = vx_true_e;
    }
    return ret;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *t)
{
    return vxReleaseReferenceInt((vx_reference *)t, VX_TYPE_THRESHOLD, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_threshold VX_API_CALL vxCreateThreshold(vx_context context, vx_enum thresh_type, vx_enum data_type)
{
    vx_threshold threshold = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (vxIsValidThresholdDataType(data_type) == vx_true_e)
        {
            if (vxIsValidThresholdType(thresh_type) == vx_true_e)
            {
                threshold = (vx_threshold)vxCreateReference(context, VX_TYPE_THRESHOLD, VX_EXTERNAL, &context->base);
                if (threshold && threshold->base.type == VX_TYPE_THRESHOLD)
                {
                    threshold->thresh_type = thresh_type;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid threshold type\n");
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_TYPE, "Invalid threshold type\n");
                threshold = (vx_threshold )vxGetErrorObject(context, VX_ERROR_INVALID_TYPE);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid data type\n");
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_TYPE, "Invalid data type\n");
            threshold = (vx_threshold )vxGetErrorObject(context, VX_ERROR_INVALID_TYPE);
        }
    }
    return threshold;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetThresholdAttribute(vx_threshold threshold, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&threshold->base, VX_TYPE_THRESHOLD) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresh_type == VX_THRESHOLD_TYPE_BINARY))
                {
                    threshold->value = *(vx_int32 *)ptr;
                    vxWroteToReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresh_type == VX_THRESHOLD_TYPE_RANGE))
                {
                    threshold->lower = *(vx_int32 *)ptr;
                    vxWroteToReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresh_type == VX_THRESHOLD_TYPE_RANGE))
                {
                    threshold->upper = *(vx_int32 *)ptr;
                    vxWroteToReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    threshold->true_value = *(vx_int32 *)ptr;
                    vxWroteToReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    threshold->false_value = *(vx_int32 *)ptr;
                    vxWroteToReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    vx_enum thresh_type = *(vx_enum *)ptr;
                    if (vxIsValidThresholdType(thresh_type) == vx_true_e)
                    {
                        threshold->thresh_type = thresh_type;
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
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    VX_PRINT(VX_ZONE_API, "return %d\n", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryThreshold(vx_threshold threshold, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&threshold->base, VX_TYPE_THRESHOLD) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresh_type == VX_THRESHOLD_TYPE_BINARY))
                {
                    *(vx_int32 *)ptr = threshold->value;
                    vxReadFromReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresh_type == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = threshold->lower;
                    vxReadFromReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresh_type == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = threshold->upper;
                    vxReadFromReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = threshold->true_value;
                    vxReadFromReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE:
                if (VX_CHECK_PARAM(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = threshold->false_value;
                    vxReadFromReference(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_ATTRIBUTE_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = threshold->thresh_type;
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
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    VX_PRINT(VX_ZONE_API, "return %d\n", status);
    return status;
}
