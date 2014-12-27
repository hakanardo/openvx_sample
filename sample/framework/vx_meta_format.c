/*
 * Copyright (c) 2014-2014 The Khronos Group Inc.
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

void vxReleaseMetaFormat(vx_meta_format *pmeta)
{
    vxReleaseReferenceInt((vx_reference *)pmeta, VX_TYPE_META_FORMAT, VX_EXTERNAL, NULL);
}

vx_meta_format vxCreateMetaFormat(vx_context context)
{
    vx_meta_format meta = (vx_meta_format)vxCreateReference(context, VX_TYPE_META_FORMAT, VX_EXTERNAL, &context->base);
    if (meta)
    {
        meta->size = sizeof(vx_meta_format_t);
        meta->type = VX_TYPE_INVALID;
    }
    return meta;
}

/******************************************************************************/
// PUBLIC
/******************************************************************************/

VX_API_ENTRY vx_status VX_API_CALL vxSetMetaFormatAttribute(vx_meta_format meta, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&meta->base, VX_TYPE_META_FORMAT) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    if (VX_TYPE(attribute) != meta->type) {
        return VX_ERROR_INVALID_TYPE;
    }

    switch(attribute) {
        /**********************************************************************/
        case VX_META_FORMAT_ATTRIBUTE_DELTA_RECTANGLE:
            if (VX_CHECK_PARAM(ptr, size, vx_delta_rectangle_t, 0x3)) {
                memcpy(&meta->dim.image.delta, ptr, size);
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_IMAGE_ATTRIBUTE_FORMAT:
            if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3)) {
                meta->dim.image.format = *(vx_df_image *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_IMAGE_ATTRIBUTE_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.image.height = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_IMAGE_ATTRIBUTE_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.image.width = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_ARRAY_ATTRIBUTE_CAPACITY:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.array.capacity = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_ARRAY_ATTRIBUTE_ITEMTYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.array.item_type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_PYRAMID_ATTRIBUTE_FORMAT:
            if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3)) {
                meta->dim.pyramid.format = *(vx_df_image *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_ATTRIBUTE_HEIGHT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.pyramid.height = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_ATTRIBUTE_WIDTH:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3)) {
                meta->dim.pyramid.width = *(vx_uint32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_ATTRIBUTE_LEVELS:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3)) {
                meta->dim.pyramid.levels = *(vx_size *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_PYRAMID_ATTRIBUTE_SCALE:
            if (VX_CHECK_PARAM(ptr, size, vx_float32, 0x3)) {
                meta->dim.pyramid.scale = *(vx_float32 *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        case VX_SCALAR_ATTRIBUTE_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3)) {
                meta->dim.scalar.type = *(vx_enum *)ptr;
            } else {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        /**********************************************************************/
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}
