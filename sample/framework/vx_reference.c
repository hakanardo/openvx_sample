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

#define VX_BAD_MAGIC (42)

vx_destructor_t destructors[] = {
        // frameworking
        {VX_TYPE_CONTEXT,       NULL},
        {VX_TYPE_GRAPH,         &vxDestructGraph},
        {VX_TYPE_NODE,          &vxDestructNode},
        {VX_TYPE_TARGET,        NULL},
        {VX_TYPE_KERNEL,        NULL}, /*!< \todo need a destructor here */
        {VX_TYPE_PARAMETER,     &vxDestructParameter},
        {VX_TYPE_ERROR,         NULL},
        {VX_TYPE_META_FORMAT,   NULL},
#ifdef OPENVX_KHR_XML
        {VX_TYPE_IMPORT,        &vxDestructImport},
#endif
        // data objects
        {VX_TYPE_ARRAY,         &vxDestructArray},
        {VX_TYPE_CONVOLUTION,   &vxDestructConvolution},
        {VX_TYPE_DISTRIBUTION,  &vxDestructDistribution},
        {VX_TYPE_DELAY,         &vxDestructDelay},
        {VX_TYPE_IMAGE,         &vxDestructImage},
        {VX_TYPE_LUT,           &vxDestructArray},
        {VX_TYPE_MATRIX,        &vxDestructMatrix},
        {VX_TYPE_SCALAR,        NULL},
        {VX_TYPE_PYRAMID,       &vxDestructPyramid},
        {VX_TYPE_REMAP,         &vxDestructRemap},
        {VX_TYPE_THRESHOLD,     NULL},
};

vx_enum static_objects[] = {
        VX_TYPE_TARGET,
        VX_TYPE_KERNEL,
};

void vxInitReference(vx_reference ref, vx_context context, vx_enum type, vx_reference scope)
{
    if (ref)
    {
        ref->context = context;
        ref->scope = scope;
        ref->magic = VX_MAGIC;
        ref->type = type;
        ref->internal_count = 0;
        ref->external_count = 0;
        ref->write_count = 0;
        ref->read_count = 0;
        ref->extracted = vx_false_e;
        ref->delay = NULL;
        ref->delay_slot_index = 0;
        ref->is_virtual = vx_false_e;
        ref->is_accessible = vx_false_e;
#ifdef OPENVX_KHR_XML
        ref->name[0] = 0;
#endif
        vxCreateSem(&ref->lock, 1);
    }
}

void vxInitReferenceForDelay(vx_reference ref, vx_delay d, vx_int32 slot_index) {
    ref->delay=d;
    ref->delay_slot_index=slot_index;
}


vx_bool vxAddReference(vx_context context, vx_reference ref)
{
    vx_uint32 r;
    vx_bool ret = vx_false_e;
    if (context)
    {
        for (r = 0; r < VX_INT_MAX_REF; r++)
        {
            if (context->reftable[r] == NULL)
            {
                context->reftable[r] = ref;
                context->num_references++;
                ret = vx_true_e;
                break;
            }
        }
    }
    else{
        /* can't add context to itself */
        ret = vx_true_e;
    }
    return ret;
}

vx_status vxReleaseReferenceInt(vx_reference *pref,
                        vx_enum type,
                        vx_enum reftype,
                        vx_destructor_f special_destructor)
{
    vx_status status = VX_SUCCESS;
    vx_reference ref = (pref ? *pref : NULL);
    if (vxIsValidSpecificReference(ref, type) == vx_true_e)
    {
        vxPrintReference(ref);
        if (vxDecrementReference(ref, reftype) == 0)
        {
            vx_uint32 d = 0u;
            vx_destructor_f destructor = special_destructor;
            vx_enum type = ref->type;

            vxRemoveReference(ref->context, ref);

            /* find the destructor method */
            if (!destructor)
            {
                for (d = 0u; d < dimof(destructors); d++)
                {
                    if (ref->type == destructors[d].type)
                    {
                        destructor = destructors[d].destructor;
                        break;
                    }
                }
            }

            /* if there is a destructor, call it. */
            if (destructor)
            {
                destructor(ref);
            }

            VX_PRINT(VX_ZONE_REFERENCE, ">>>> Reference count was zero, destructed object "VX_FMT_REF"\n", ref);

            vxDestroySem(&ref->lock);
            ref->magic = VX_BAD_MAGIC; /* make sure no existing copies of refs can use ref again */

            /* some objects are statically allocated. */
            for (d = 0; d < dimof(static_objects); d++) {
                if (type == static_objects[d])
                    break;
            }
            if (d == dimof(static_objects)) { // not found in list
                free(ref);
            }
        }
        *pref = NULL;
    } else {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

typedef struct _vx_type_size {
    vx_enum type;
    vx_size size;
} vx_type_size_t;

static vx_type_size_t type_sizes[] = {
    {VX_TYPE_INVALID,   0},
    // scalars
    {VX_TYPE_CHAR,      sizeof(vx_char)},
    {VX_TYPE_INT8,      sizeof(vx_int8)},
    {VX_TYPE_INT16,     sizeof(vx_int16)},
    {VX_TYPE_INT32,     sizeof(vx_int32)},
    {VX_TYPE_INT64,     sizeof(vx_int64)},
    {VX_TYPE_UINT8,     sizeof(vx_uint8)},
    {VX_TYPE_UINT16,    sizeof(vx_uint16)},
    {VX_TYPE_UINT32,    sizeof(vx_uint32)},
    {VX_TYPE_UINT64,    sizeof(vx_uint64)},
    {VX_TYPE_FLOAT32,   sizeof(vx_float32)},
    {VX_TYPE_FLOAT64,   sizeof(vx_float64)},
    {VX_TYPE_ENUM,      sizeof(vx_enum)},
    {VX_TYPE_BOOL,      sizeof(vx_bool)},
    {VX_TYPE_SIZE,      sizeof(vx_size)},
    {VX_TYPE_DF_IMAGE,    sizeof(vx_df_image)},
    // structures
    {VX_TYPE_RECTANGLE,     sizeof(vx_rectangle_t)},
    {VX_TYPE_COORDINATES2D, sizeof(vx_coordinates2d_t)},
    {VX_TYPE_COORDINATES3D, sizeof(vx_coordinates3d_t)},
    {VX_TYPE_KEYPOINT,      sizeof(vx_keypoint_t)},
    // pseudo objects
    {VX_TYPE_ERROR,     sizeof(vx_error_t)},
    {VX_TYPE_META_FORMAT,sizeof(vx_meta_format_t)},
    // framework objects
    {VX_TYPE_REFERENCE, sizeof(vx_reference_t)},
    {VX_TYPE_CONTEXT,   sizeof(vx_context_t)},
    {VX_TYPE_GRAPH,     sizeof(vx_graph_t)},
    {VX_TYPE_NODE,      sizeof(vx_node_t)},
    {VX_TYPE_TARGET,    sizeof(vx_target_t)},
    {VX_TYPE_PARAMETER, sizeof(vx_parameter_t)},
    {VX_TYPE_KERNEL,    sizeof(vx_kernel_t)},
    // data objects
    {VX_TYPE_ARRAY,     sizeof(vx_array_t)},
    {VX_TYPE_CONVOLUTION, sizeof(vx_convolution_t)},
    {VX_TYPE_DELAY,     sizeof(vx_delay_t)},
    {VX_TYPE_DISTRIBUTION, sizeof(vx_distribution_t)},
    {VX_TYPE_IMAGE,     sizeof(vx_image_t)},
    {VX_TYPE_LUT,       sizeof(vx_lut_t)},
    {VX_TYPE_MATRIX,    sizeof(vx_matrix_t)},
    {VX_TYPE_PYRAMID,   sizeof(vx_pyramid_t)},
    {VX_TYPE_REMAP,     sizeof(vx_remap_t)},
    {VX_TYPE_SCALAR,    sizeof(vx_scalar_t)},
    {VX_TYPE_THRESHOLD, sizeof(vx_threshold_t)},
#ifdef OPENVX_KHR_XML
    {VX_TYPE_IMPORT,    sizeof(vx_import_t)},
#endif
    // others
};

vx_size vxSizeOfType(vx_enum type)
{
    vx_uint32 i = 0;
    vx_size size = 0ul;
    for (i = 0; i < dimof(type_sizes); i++) {
        if (type == type_sizes[i].type) {
            size = type_sizes[i].size;
            break;
        }
    }
    return size;
}


vx_reference vxCreateReference(vx_context context, vx_enum type, vx_enum reftype, vx_reference scope)
{
    vx_size size = vxSizeOfType(type);
    vx_reference ref = (vx_reference)calloc(1, size);
    if (ref)
    {
        vxInitReference(ref, context, type, scope);
        vxIncrementReference(ref, reftype);
        if (vxAddReference(context, ref) == vx_false_e)
        {
            free(ref);
            VX_PRINT(VX_ZONE_ERROR, "Failed to add reference to global table!\n");
            vxAddLogEntry(&context->base, VX_ERROR_NO_RESOURCES, "Failed to add to resources table\n");
            ref = (vx_reference)vxGetErrorObject(context, VX_ERROR_NO_RESOURCES);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to allocate memory\n");
        vxAddLogEntry(&context->base, VX_ERROR_NO_MEMORY, "Failed to allocate memory\n");
        ref = (vx_reference)vxGetErrorObject(context, VX_ERROR_NO_MEMORY);
    }
    return ref;
}

vx_bool vxIsValidReference(vx_reference ref)
{
    vx_bool ret = vx_false_e;
    if (ref != NULL)
    {
        vxPrintReference(ref);
        if ((ref->magic == VX_MAGIC) &&
            (vxIsValidType(ref->type) && ref->type != VX_TYPE_CONTEXT) &&
            (vxIsValidContext(ref->context) == vx_true_e))
        {
            ret = vx_true_e;
        }
        else if (ref->magic == VX_BAD_MAGIC)
        {
            VX_PRINT(VX_ZONE_ERROR, "%p has already been released and garbage collected!\n", ref);
        }
        else if (ref->type != VX_TYPE_CONTEXT)
        {
            VX_PRINT(VX_ZONE_ERROR, "%p is not a valid reference!\n", ref);
            DEBUG_BREAK();
            VX_BACKTRACE(VX_ZONE_ERROR);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Reference was NULL\n");
    }
    return ret;
}

vx_bool vxIsValidSpecificReference(vx_reference ref, vx_enum type)
{
    vx_bool ret = vx_false_e;
    if (ref != NULL)
    {
        //vxPrintReference(ref);
        if ((ref->magic == VX_MAGIC) &&
            (ref->type == type) &&
            (vxIsValidContext(ref->context) == vx_true_e))
        {
            ret = vx_true_e;
        }
        else if (ref->type != VX_TYPE_CONTEXT)
        {
            VX_PRINT(VX_ZONE_ERROR, "%p is not a valid reference!\n", ref);
            //DEBUG_BREAK(); // this will catch any "invalid" objects, but is not useful otherwise.
            VX_BACKTRACE(VX_ZONE_WARNING);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_WARNING, "Reference was NULL\n");
        //DEBUG_BREAK();
        VX_BACKTRACE(VX_ZONE_WARNING);
    }
    return ret;
}

vx_bool vxRemoveReference(vx_context context, vx_reference ref)
{
    vx_uint32 r;
    for (r = 0; r < VX_INT_MAX_REF; r++)
    {
        if (context->reftable[r] == ref)
        {
            context->reftable[r] = NULL;
            context->num_references--;
            return vx_true_e;
        }
    }
    return vx_false_e;
}

void vxPrintReference(vx_reference ref)
{
    if (ref)
    {
        VX_PRINT(VX_ZONE_REFERENCE, "vx_reference_t:%p magic:%08x type:%08x count:[%u,%u] context:%p\n", ref, ref->magic, ref->type, ref->external_count, ref->internal_count, ref->context);
    }
}


vx_uint32 vxIncrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 count = 0u;
    if (ref)
    {
        vxSemWait(&ref->lock);
        if (reftype == VX_EXTERNAL || reftype == VX_BOTH)
            ref->external_count++;
        if (reftype == VX_INTERNAL || reftype == VX_BOTH)
            ref->internal_count++;
        count = ref->internal_count + ref->external_count;
        VX_PRINT(VX_ZONE_REFERENCE, "Incremented Total Reference Count to %u on "VX_FMT_REF"\n", count, ref);
        vxSemPost(&ref->lock);
    }
    return count;
}

vx_uint32 vxDecrementReference(vx_reference ref, vx_enum reftype)
{
    vx_uint32 result = UINT32_MAX;
    if (ref)
    {
        vxSemWait(&ref->lock);
        if (reftype == VX_INTERNAL || reftype == VX_BOTH) {
            if (ref->internal_count == 0) {
                VX_PRINT(VX_ZONE_WARNING, "#### INTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
                DEBUG_BREAK();
            } else {
                ref->internal_count--;
            }
        }
        if (reftype == VX_EXTERNAL || reftype == VX_BOTH) {
            if (ref->external_count == 0)
            {
                VX_PRINT(VX_ZONE_WARNING, "#### EXTERNAL REF COUNT IS ALREADY ZERO!!! "VX_FMT_REF" type:%08x #####\n", ref, ref->type);
                DEBUG_BREAK();
            }
            else
            {
                ref->external_count--;
                if ((ref->external_count == 0) && (ref->extracted == vx_true_e))
                {
                    ref->extracted = vx_false_e;
                }
            }
        }
        result = ref->internal_count + ref->external_count;
        VX_PRINT(VX_ZONE_REFERENCE, "Decremented Total Reference Count to %u on "VX_FMT_REF" type:%08x\n", result, ref, ref->type);
        vxSemPost(&ref->lock);
    }
    return result;
}

vx_uint32 vxTotalReferenceCount(vx_reference ref)
{
    vx_uint32 count = 0;
    if (ref)
    {
        vxSemWait(&ref->lock);
        count = ref->external_count + ref->internal_count;
        vxSemPost(&ref->lock);
    }
    return count;
}

void vxWroteToReference(vx_reference ref)
{
    if (ref)
    {
        vxSemWait(&ref->lock);
        ref->write_count++;
        if (ref->extracted == vx_true_e)
        {
            vxContaminateGraphs(ref);
        }
        vxSemPost(&ref->lock);
    }
}

void vxReadFromReference(vx_reference ref)
{
    if (ref)
    {
        vxSemWait(&ref->lock);
        ref->read_count++;
        vxSemPost(&ref->lock);
    }
}

/*****************************************************************************/
// PUBLIC
/*****************************************************************************/

VX_API_ENTRY vx_status VX_API_CALL vxQueryReference(vx_reference ref, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    /* if it is not a reference and not a context */
    if ((vxIsValidReference(ref) == vx_false_e) &&
        (vxIsValidContext((vx_context_t *)ref) == vx_false_e)) {
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_REF_ATTRIBUTE_COUNT:
            if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
            {
                *(vx_uint32 *)ptr = ref->external_count;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_REF_ATTRIBUTE_TYPE:
            if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
            {
                *(vx_enum *)ptr = ref->type;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
#ifdef OPENVX_KHR_XML
        case VX_REF_ATTRIBUTE_NAME:
            if ((size <= VX_MAX_REFERENCE_NAME) && (ptr != NULL))
            {
                strncpy(ptr, ref->name, size);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
#endif
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}

