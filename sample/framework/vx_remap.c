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

VX_API_ENTRY vx_remap VX_API_CALL vxCreateRemap(vx_context context,
                              vx_uint32 src_width, vx_uint32 src_height,
                              vx_uint32 dst_width, vx_uint32 dst_height)
{
    vx_remap remap = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (src_width != 0 && src_height != 0 && dst_width != 0 && dst_height != 0)
        {
            remap = (vx_remap)vxCreateReference(context, VX_TYPE_REMAP, VX_EXTERNAL, &context->base);
            if (remap && remap->base.type == VX_TYPE_REMAP)
            {
                remap->src_width = src_width;
                remap->src_height = src_height;
                remap->dst_width = dst_width;
                remap->dst_height = dst_height;
                remap->memory.ndims = 3;
                remap->memory.nptrs = 1;
                remap->memory.dims[0][VX_DIM_C] = 2; // 2 "channels" of f32
                remap->memory.dims[0][VX_DIM_X] = dst_width;
                remap->memory.dims[0][VX_DIM_Y] = dst_height;
                remap->memory.strides[0][VX_DIM_C] = sizeof(vx_float32);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid parameters to remap\n");
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Invalid parameters to remap\n");
            remap = (vx_remap_t *)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
    }
    return remap;
}

void vxDestructRemap(vx_reference ref)
{
    vx_remap remap = (vx_remap_t *)ref;
    vxFreeMemory(remap->base.context, &remap->memory);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseRemap(vx_remap *r)
{
    return vxReleaseReferenceInt((vx_reference *)r, VX_TYPE_REMAP, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryRemap(vx_remap remap, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&remap->base, VX_TYPE_REMAP) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_REMAP_ATTRIBUTE_SOURCE_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32 *)ptr = remap->src_width;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REMAP_ATTRIBUTE_SOURCE_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32 *)ptr = remap->src_height;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REMAP_ATTRIBUTE_DESTINATION_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32 *)ptr = remap->dst_width;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REMAP_ATTRIBUTE_DESTINATION_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32 *)ptr = remap->dst_height;
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

VX_API_ENTRY vx_status VX_API_CALL vxSetRemapPoint(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 src_x, vx_float32 src_y)
{
    vx_status status = VX_FAILURE;
    if ((vxIsValidSpecificReference(&remap->base, VX_TYPE_REMAP) == vx_true_e) &&
         (vxAllocateMemory(remap->base.context, &remap->memory) == vx_true_e))
    {
        if ((dst_x < remap->dst_width) &&
            (dst_y < remap->dst_height))
        {
            vx_float32 *coords[] = {
                 vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0),
                 vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0),
            };
            *coords[0] = src_x;
            *coords[1] = src_y;
            vxWroteToReference(&remap->base);
            status = VX_SUCCESS;
            VX_PRINT(VX_ZONE_INFO, "SetRemapPoint %ux%u to %f,%f\n", dst_x, dst_y, src_x, src_y);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid source or destintation values!\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid object!\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetRemapPoint(vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y,
                                 vx_float32 *src_x, vx_float32 *src_y)
{
    vx_status status = VX_FAILURE;
    if (vxIsValidSpecificReference(&remap->base, VX_TYPE_REMAP) == vx_true_e)
    {
        if ((dst_x < remap->dst_width) &&
            (dst_y < remap->dst_height))
        {
            vx_float32 *coords[] = {
                 vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0),
                 vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0),
            };
            *src_x = *coords[0];
            *src_y = *coords[1];
            remap->base.read_count++;
            status = VX_SUCCESS;
            VX_PRINT(VX_ZONE_INFO, "GetRemapPoint dst[%u,%u] to src[%f,%f]\n", dst_x, dst_y, src_x, src_y);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid source or destintation values!\n");
            status = VX_ERROR_INVALID_VALUE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid object!\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}
