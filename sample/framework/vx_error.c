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

void vxReleaseErrorInt(vx_error_t **error)
{
    vxReleaseReferenceInt((vx_reference *)error, VX_TYPE_ERROR, VX_INTERNAL, NULL);
}

vx_error_t *vxAllocateError(vx_context_t *context, vx_status status)
{
    vx_error_t *error = (vx_error_t *)vxCreateReference(context, VX_TYPE_ERROR, VX_INTERNAL, &context->base);
    if (error)
    {
        error->status = status;
    }
    return error;
}

vx_bool vxCreateConstErrors(vx_context_t *context)
{
    vx_bool ret = vx_true_e;
    vx_enum e = 0;
    /* create an error object for each status enumeration */
    for (e = VX_STATUS_MIN; (e < VX_SUCCESS) && (ret == vx_true_e); e++)
    {
        if (vxAllocateError(context, e) == NULL)
            ret = vx_false_e;
    }
    return ret;
}

vx_error_t *vxGetErrorObject(vx_context_t *context, vx_status status)
{
    vx_error_t *error = NULL;
    vx_size i = 0ul;
    for (i = 0ul; i < context->num_references; i++)
    {
        if (context->reftable[i] == NULL)
            continue;

        if (context->reftable[i]->type == VX_TYPE_ERROR)
        {
            error = (vx_error_t *)context->reftable[i];
            if (error->status == status)
            {
                break;
            }
            error = NULL;
        }
    }
    return error;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetStatus(vx_reference reference)
{
    vx_reference_t *ref = (vx_reference_t *)reference;
    if (ref == NULL)
    {
        /*! \internal probably ran out of handles or memory */
        return VX_ERROR_NO_RESOURCES;
    }
    else if (vxIsValidReference(ref) == vx_true_e)
    {
        if (ref->type == VX_TYPE_ERROR)
        {
            vx_error_t *error = (vx_error_t *)ref;
            return error->status;
        }
        else
        {
            return VX_SUCCESS;
        }
    }
    else if (vxIsValidContext((vx_context_t *)ref) == vx_true_e)
    {
        return VX_SUCCESS;
    }
    return VX_FAILURE;
}
