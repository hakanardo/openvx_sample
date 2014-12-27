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

#define vxIsValidDelay(d) vxIsValidSpecificReference((vx_reference)(d), VX_TYPE_DELAY)

vx_bool vxAddAssociationToDelay(vx_reference value, vx_node n, vx_uint32 i)
{

    vx_delay delay = value->delay;
    vx_int32 delay_index = value->delay_slot_index;

    vx_int32 index = (delay->index + delay->count - abs(delay_index)) % (vx_int32)delay->count;

    if (delay->set[index].node == 0) // head is empty
    {
        delay->set[index].node = n;
        delay->set[index].index = i;
    }
    else
    {
        vx_delay_param_t **ptr = &delay->set[index].next;
        do {
            if (*ptr == NULL)
            {
                *ptr = VX_CALLOC(vx_delay_param_t);
                if (*ptr != NULL)
                {
                    (*ptr)->node = n;
                    (*ptr)->index = i;
                    break;
                }
                else {
                    return vx_false_e;
                }
            }
            else {
                ptr = &((*ptr)->next);
            }
        } while (1);
    }

    // Increment a reference to the delay
    vxIncrementReference((vx_reference)delay, VX_INTERNAL);

    return vx_true_e;
}

vx_bool vxRemoveAssociationToDelay(vx_reference value, vx_node n, vx_uint32 i)
{
    vx_delay delay = value->delay;
    vx_int32 delay_index = value->delay_slot_index;

    vx_int32 index = (delay->index + delay->count - abs(delay_index)) % (vx_int32)delay->count;

    if (index >= delay->count) {
        return vx_false_e;
    }

    if (delay->set[index].node == n && delay->set[index].index == i) // head is a match
    {
        delay->set[index].node = 0;
        delay->set[index].index = 0;
    }
    else
    {
        vx_delay_param_t **ptr = &delay->set[index].next;
        vx_delay_param_t *next = NULL;
        do {
            if (*ptr != NULL)
            {
                if ((*ptr)->node == n && (*ptr)->index == i)
                {
                    (*ptr)->node = 0;
                    (*ptr)->index = 0;
                    next = (*ptr)->next;
                    free(*ptr);
                    *ptr = next;
                    break;
                }
                else {
                   ptr = &((*ptr)->next);
                }
            }
            else {
                return vx_false_e;
            }
        } while (1);
    }

    // Release the delay
    {
        vx_reference ref=(vx_reference)delay;
        vxReleaseReferenceInt(&ref, VX_TYPE_DELAY, VX_INTERNAL, NULL);
    }
    return vx_true_e;
}




/******************************************************************************/
/* PUBLIC INTERFACE */
/******************************************************************************/

VX_API_ENTRY vx_reference VX_API_CALL vxGetReferenceFromDelay(vx_delay delay, vx_int32 index)
{
    vx_reference ref = 0;
    if (vxIsValidDelay(delay) == vx_true_e)
    {
        if ((vx_uint32)abs(index) < delay->count)
        {
            vx_int32 i = (delay->index + abs(index)) % (vx_int32)delay->count;
            ref = delay->refs[i];
            VX_PRINT(VX_ZONE_DELAY, "Retrieving relative index %d => " VX_FMT_REF  " from Delay (%d)\n", index, ref, i);
        }
    }
    return ref;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryDelay(vx_delay delay, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidDelay(delay) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_DELAY_ATTRIBUTE_TYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                    *(vx_enum *)ptr = delay->type;
                else
                    status = VX_ERROR_INVALID_PARAMETERS;
                break;
            case VX_DELAY_ATTRIBUTE_COUNT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                    *(vx_uint32 *)ptr = (vx_uint32)delay->count;
                else
                    status = VX_ERROR_INVALID_PARAMETERS;
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
    VX_PRINT(VX_ZONE_API, "%s returned %d\n",__FUNCTION__, status);
    return status;
}

void vxDestructDelay(vx_reference ref) {
    vx_uint32 i = 0;
    vx_delay delay = (vx_delay)ref;
    for (i = 0; i < delay->count; i++)
    {
        vxReleaseReferenceInt(&delay->refs[i], delay->type, VX_INTERNAL, NULL);
    }
    if (delay->set)
    {
        vx_uint32 i;
        for (i = 0; i < delay->count; i++)
        {
            vx_delay_param_t *cur = delay->set[i].next;
            while (cur != NULL)
            {
                vx_delay_param_t *next = cur->next;
                free(cur);
                cur = next;
            }
        }
        free(delay->set);
    }
    if (delay->refs) {
        free(delay->refs);
    }
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseDelay(vx_delay *d)
{
    return vxReleaseReferenceInt((vx_reference *)d, VX_TYPE_DELAY, VX_EXTERNAL, &vxDestructDelay);
}

VX_API_ENTRY vx_delay VX_API_CALL vxCreateDelay(vx_context context,
                              vx_reference exemplar,
                              vx_size count)
{
    vx_delay delay = NULL;
    vx_enum invalid_types[] = {
        VX_TYPE_CONTEXT,
        VX_TYPE_GRAPH,
        VX_TYPE_NODE,
        VX_TYPE_KERNEL,
        VX_TYPE_TARGET,
        VX_TYPE_PARAMETER,
        VX_TYPE_REFERENCE,
        VX_TYPE_DELAY, // no delays of delays
#ifdef OPENVX_KHR_XML
        VX_TYPE_IMPORT,
#endif
    };
    vx_uint32 t = 0u;

    if (vxIsValidContext(context) == vx_false_e)
        return 0;

    if (vxIsValidReference(exemplar) == vx_false_e)
        return (vx_delay)vxGetErrorObject(context, VX_ERROR_INVALID_REFERENCE);

    for (t = 0u; t < dimof(invalid_types); t++)
    {
        if (exemplar->type == invalid_types[t])
        {
            VX_PRINT(VX_ZONE_ERROR, "Attempted to create delay of invalid object type!\n");
            vxAddLogEntry((vx_reference)context, VX_ERROR_INVALID_REFERENCE, "Attempted to create delay of invalid object type!\n");
            return (vx_delay)vxGetErrorObject(context, VX_ERROR_INVALID_REFERENCE);
        }
    }

    delay = (vx_delay)vxCreateReference(context, VX_TYPE_DELAY, VX_EXTERNAL, &context->base);
    if (delay && delay->base.type == VX_TYPE_DELAY)
    {
        vx_size i = 0;
        delay->set = (vx_delay_param_t *)calloc(count, sizeof(vx_delay_param_t));
        delay->refs = (vx_reference *)calloc(count, sizeof(vx_reference));
        delay->type = exemplar->type;
        delay->count = count;
        VX_PRINT(VX_ZONE_DELAY, "Creating Delay of %u objects of type %x!\n", count, exemplar->type);
        for (i = 0; i < count; i++)
        {
            switch (exemplar->type)
            {
                case VX_TYPE_IMAGE:
                {
                    vx_image image = (vx_image )exemplar;
                    delay->refs[i] = (vx_reference)vxCreateImage(context, image->width, image->height, image->format);
                    break;
                }
                case VX_TYPE_ARRAY:
                {
                    vx_array arr = (vx_array )exemplar;
                    delay->refs[i] = (vx_reference)vxCreateArray(context, arr->item_type, arr->capacity);
                    break;
                }
                case VX_TYPE_MATRIX:
                {
                    vx_matrix mat = (vx_matrix)exemplar;
                    delay->refs[i] = (vx_reference)vxCreateMatrix(context, mat->data_type, mat->columns, mat->rows);
                    break;
                }
                case VX_TYPE_CONVOLUTION:
                {
                    vx_convolution conv = (vx_convolution)exemplar;
                    vx_convolution conv2 = vxCreateConvolution(context, conv->base.columns, conv->base.rows);
                    conv2->scale = conv->scale;
                    delay->refs[i] = (vx_reference)conv2;
                    break;
                }
                case VX_TYPE_DISTRIBUTION:
                {
                    vx_distribution dist = (vx_distribution)exemplar;
                    vx_uint32 range = dist->memory.dims[0][VX_DIM_X] * dist->window_x;
                    delay->refs[i] = (vx_reference)vxCreateDistribution(context, dist->memory.dims[0][VX_DIM_X], dist->offset_x, range);
                    break;
                }
                case VX_TYPE_REMAP:
                {
                    vx_remap remap = (vx_remap)exemplar;
                    delay->refs[i] = (vx_reference)vxCreateRemap(context, remap->src_width, remap->src_height, remap->dst_width, remap->dst_height);
                    break;
                }
                case VX_TYPE_LUT:
                {
                    vx_lut_t *lut = (vx_lut_t *)exemplar;
                    delay->refs[i] = (vx_reference)vxCreateLUT(context, lut->item_type, lut->capacity);
                    break;
                }
                case VX_TYPE_PYRAMID:
                {
                    vx_pyramid pyramid = (vx_pyramid )exemplar;
                    delay->refs[i] = (vx_reference)vxCreatePyramid(context, pyramid->numLevels, pyramid->scale, pyramid->width, pyramid->height, pyramid->format);
                    break;
                }
                case VX_TYPE_THRESHOLD:
                {
                    vx_threshold thresh = (vx_threshold )exemplar;
                    delay->refs[i] = (vx_reference)vxCreateThreshold(context, thresh->thresh_type, VX_TYPE_UINT8);
                    break;
                }
                case VX_TYPE_SCALAR:
                {
                    vx_scalar scalar = (vx_scalar )exemplar;
                    delay->refs[i] = (vx_reference)vxCreateScalar(context, scalar->data_type, NULL);
                    break;
                }
                default:
                    break;
            }
            /* set the object as a delay element */
            vxInitReferenceForDelay(delay->refs[i], delay, i);
            /* change the counting from external to internal */
            vxIncrementReference(delay->refs[i], VX_INTERNAL);
            vxDecrementReference(delay->refs[i], VX_EXTERNAL);
            /* set the scope to the delay */
            ((vx_reference )delay->refs[i])->scope = (vx_reference )delay;
        }
    }
    return (vx_delay)delay;
}

VX_API_ENTRY vx_status VX_API_CALL vxAgeDelay(vx_delay delay)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidDelay(delay) == vx_true_e)
    {
        vx_int32 i,j;

        // increment the index
        delay->index = (delay->index + 1) % (vx_uint32)delay->count;

        VX_PRINT(VX_ZONE_DELAY, "Delay has shifted by 1, base index is now %d\n", delay->index);

        // then reassign the parameters
        for (i = 0; i < delay->count; i++)
        {
            vx_delay_param_t *param = NULL;

            j = (delay->index + i) % (vx_int32)delay->count;
            param = &delay->set[i];
            do {
                if (param->node != 0)
                {
                    vxNodeSetParameter(param->node,
                                       param->index,
                                       delay->refs[j]);
                }
                param = param->next;
            } while (param != NULL);
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}
