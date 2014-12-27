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

VX_API_ENTRY vx_lut VX_API_CALL vxCreateLUT(vx_context context, vx_enum data_type, vx_size count)
{
    vx_lut_t *lut = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (data_type == VX_TYPE_UINT8)
        {
#if defined(OPENVX_STRICT_1_0)
            if (count != 256)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid parameter to LUT\n");
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Invalid parameter to LUT\n");
                lut = (vx_lut_t *)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
            }
            else
#endif
            {
                lut = (vx_lut_t *)vxCreateArrayInt(context, VX_TYPE_UINT8, count, vx_false_e, VX_TYPE_LUT);
                if (lut && lut->base.type == VX_TYPE_LUT)
                {
                    lut->num_items = count;
                    vxPrintArray(lut);
                }
            }
        }
#if !defined(OPENVX_STRICT_1_0)
        else if (data_type == VX_TYPE_UINT16)
        {
            lut = (vx_lut_t *)vxCreateArrayInt(context, VX_TYPE_UINT16, count, vx_false_e, VX_TYPE_LUT);
            if (lut && lut->base.type == VX_TYPE_LUT)
            {
                lut->num_items = count;
                vxPrintArray(lut);
            }
        }
#endif
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid data type\n");
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_TYPE, "Invalid data type\n");
            lut = (vx_lut_t *)vxGetErrorObject(context, VX_ERROR_INVALID_TYPE);
        }
    }
    return (vx_lut)lut;
}

void vxDestructLUT(vx_reference ref)
{
    vx_lut_t *lut = (vx_lut_t *)ref;
    vxDestructArray((vx_reference_t *)lut);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseLUT(vx_lut *l)
{
    return vxReleaseReferenceInt((vx_reference *)l, VX_TYPE_LUT, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryLUT(vx_lut l, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    vx_lut_t *lut = (vx_lut_t *)l;

    if (vxIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_LUT_ATTRIBUTE_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
            {
                *(vx_enum *)ptr = lut->item_type;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_LUT_ATTRIBUTE_COUNT:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = lut->num_items;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_LUT_ATTRIBUTE_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                *(vx_size *)ptr = lut->num_items * lut->item_size;
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

VX_API_ENTRY vx_status VX_API_CALL vxAccessLUT(vx_lut l, void **ptr, vx_enum usage)
{
    vx_status status = VX_FAILURE;
    vx_lut_t *lut = (vx_lut_t *)l;
    if (vxIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_true_e)
    {
        status = vxAccessArrayRangeInt((vx_array_t *)l, 0, lut->num_items, ptr, usage);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid object!\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitLUT(vx_lut l, void *ptr)
{
    vx_status status = VX_FAILURE;
    vx_lut_t *lut = (vx_lut_t *)l;
    if (vxIsValidSpecificReference(&lut->base, VX_TYPE_LUT) == vx_true_e)
    {
        status = vxCommitArrayRangeInt((vx_array_t *)l, 0, lut->num_items, ptr);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid object!\n");
    }
    return status;
}

