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

#if defined(EXPERIMENTAL_USE_XML)

vx_import vxCreateImportInt(vx_context context,
                              vx_enum type,
                              vx_uint32 count)
{
    vx_import import = NULL;

    if (vxIsValidContext(context) == vx_false_e)
        return 0;

    import = (vx_import)vxCreateReference(context, VX_TYPE_IMPORT, VX_EXTERNAL, &context->base);
    if (import && import->base.type == VX_TYPE_IMPORT)
    {
        import->refs = (vx_reference *)calloc(count, sizeof(vx_reference));
        import->type = type;
        import->count = count;
        VX_PRINT(VX_ZONE_INFO, "Creating Import of %u objects of type %x!\n", count, type);
    }
    return import;
}

void vxDestructImport(vx_reference ref) {
    vx_uint32 i = 0;
    vx_import import = (vx_import)ref;
    for (i = 0; i < import->count; i++)
    {
        vxReleaseReferenceInt(&import->refs[i], import->refs[i]->type, VX_INTERNAL, NULL);
    }
    if (import->refs) {
        free(import->refs);
    }
}


/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/

VX_API_ENTRY vx_status VX_API_CALL vxSetReferenceName(vx_reference ref, const vx_char *name)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidReference(ref))
    {
        strncpy(ref->name, name, strnlen(name, VX_MAX_REFERENCE_NAME));
        status = VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceByIndex(vx_import import, vx_uint32 index)
{
    vx_reference ref = NULL;
    if (import && import->base.type == VX_TYPE_IMPORT)
    {
        if (index < import->count)
        {
            ref = (vx_reference_t *)import->refs[index];
            vxIncrementReference(ref, VX_EXTERNAL);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect index value\n");
            vxAddLogEntry(&import->base.context->base, VX_ERROR_INVALID_PARAMETERS, "Incorrect index value\n");
            ref = (vx_reference_t *)vxGetErrorObject(import->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid import reference!\n");
    }
    return ref;
}

VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceByName(vx_import import, const vx_char *name)
{
    vx_reference ref = NULL;
    if (import && import->base.type == VX_TYPE_IMPORT)
    {
        vx_uint32 index = 0;
        for (index = 0; index < import->count; index++)
        {
            if (strncmp(name, import->refs[index]->name, VX_MAX_REFERENCE_NAME) == 0)
            {
                ref = (vx_reference_t*)import->refs[index];
                vxIncrementReference(ref, VX_EXTERNAL);
                break;
            }
        }
    }
    return ref;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryImport(vx_import import, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference((vx_reference_t *)import, VX_TYPE_IMPORT) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_IMPORT_ATTRIBUTE_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = import->count;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMPORT_ATTRIBUTE_TYPE:
                if ((size <= VX_MAX_TARGET_NAME) && (ptr != NULL))
                {
                    *(vx_uint32 *)ptr = import->type;
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
        status = VX_ERROR_INVALID_REFERENCE;
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseImport(vx_import *import)
{
    return vxReleaseReferenceInt((vx_reference *)import, VX_TYPE_IMPORT, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseReference(vx_reference *ref)
{
    if(vxIsValidReference(*ref))
    {
        return vxReleaseReferenceInt((vx_reference *)ref, (*ref)->type, VX_EXTERNAL, NULL);
    } else {
        return VX_ERROR_INVALID_REFERENCE;
    }
}

#endif

