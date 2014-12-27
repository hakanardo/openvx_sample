/*
 * Copyright (c) 2011-2014 The Khronos Group Inc.
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

 #include "vx_internal.h"

/*==============================================================================
 PRIVATE INTERFACE
 =============================================================================*/

static vx_size vxArrayItemSize(vx_context context, vx_enum item_type)
{
    vx_size res = vxSizeOfType(item_type);
    vx_uint32 i = 0;
    if (res == 0ul)
    {
        for (i = 0; i < VX_INT_MAX_USER_STRUCTS; ++i)
        {
            if (context->user_structs[i].type == item_type)
            {
                res = context->user_structs[i].size;
                break;
            }
        }
    }
    return res;
}

static vx_bool vxIsValidArrayItemType(vx_context context, vx_enum item_type)
{
    vx_bool res = vx_false_e;

    if (vxArrayItemSize(context, item_type) != 0)
    {
        res = vx_true_e;
    }

    return res;
}

static vx_bool vxIsValidArray(vx_array arr)
{
    vx_bool res = vx_false_e;

    if (arr != NULL)
        res = vxIsValidSpecificReference(&arr->base, VX_TYPE_ARRAY);

    if (res == vx_true_e)
        res = vxIsValidArrayItemType(arr->base.context, arr->item_type);

    return res;
}

static void vxInitArrayMemory(vx_array arr)
{
    arr->memory.nptrs = 1;
    arr->memory.ndims = 2;

    arr->memory.dims[0][0] = (int32_t)arr->item_size;
    arr->memory.dims[0][1] = (int32_t)arr->capacity;
}

/*==============================================================================
 INTERNAL INTERFACE
 =============================================================================*/

void vxPrintArray(vx_array array)
{
    VX_PRINT(VX_ZONE_INFO, "Array:%p has %zu elements of %04x type of %zu size each.\n", array, array->capacity, array->item_type, array->item_size);
}

vx_array vxCreateArrayInt(vx_context context, vx_enum item_type, vx_size capacity, vx_bool is_virtual, vx_enum type)
{
    vx_array arr = (vx_array)vxCreateReference(context, type, VX_EXTERNAL, &context->base);
    if (arr && arr->base.type == type)
    {
        arr->item_type = item_type;
        arr->item_size = vxArrayItemSize(context, item_type);
        arr->capacity = capacity;
        arr->base.is_virtual = is_virtual;
        vxInitArrayMemory(arr);
    }
    return arr;
}

void vxDestructArray(vx_reference ref)
{
    vx_array arr = (vx_array)ref;
    vxFreeMemory(arr->base.context, &arr->memory);
}

vx_bool vxInitVirtualArray(vx_array arr, vx_enum item_type, vx_size capacity)
{
    vx_bool res = vx_false_e;
    if ((vxIsValidArrayItemType(arr->base.context, item_type) == vx_true_e) &&
        ((arr->capacity > 0 || capacity > 0) && (capacity <= arr->capacity || arr->capacity == 0)))
    {
        if ((arr->item_type == VX_TYPE_INVALID) || (arr->item_type == item_type))
        {
            arr->item_type = item_type;
            arr->item_size = vxArrayItemSize(arr->base.context, item_type);

            if (arr->capacity == 0)
                arr->capacity = capacity;

            vxInitArrayMemory(arr);
            res = vx_true_e;
        }
    }
    return res;
}

vx_bool vxValidateArray(vx_array arr, vx_enum item_type, vx_size capacity)
{
    vx_bool res = vx_false_e;
    if ((vxIsValidArrayItemType(arr->base.context, item_type)) &&
        (arr->item_type == item_type))
    {
        /* if the required capacity is > 0 and the objects capacity is not sufficient */
        if ((capacity > 0) && (capacity > arr->capacity))
            res = vx_false_e;
        else
            res = vx_true_e;
    }
    return res;
}

vx_bool vxAllocateArray(vx_array arr)
{
    vx_bool res = vx_false_e;
    if (arr->capacity > 0)
    {
        res = vxAllocateMemory(arr->base.context, &arr->memory);
    }
    return res;
}

vx_status vxAccessArrayRangeInt(vx_array arr, vx_size start, vx_size end, void **ptr, vx_enum usage)
{
    vx_status status = VX_FAILURE;
    vx_bool mapped = vx_false_e;

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) ||
        (start >= end) || (end > arr->num_items))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* determine if virtual before checking for memory */
    if (arr->base.is_virtual == vx_true_e)
    {
        if (arr->base.is_accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            VX_PRINT(VX_ZONE_ERROR, "Can not access a virtual array\n");
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    /* verify has not run or will not run yet. this allows this API to "touch"
     * the array to create it.
     */
    if (vxAllocateArray(arr) == vx_false_e)
    {
        return VX_ERROR_NO_MEMORY;
    }

    /* POSSIBILITIES:
     * 1.) !*ptr && RO == COPY-ON-READ (make ptr=alloc)
     * 2.) !*ptr && WO == MAP
     * 3.) !*ptr && RW == MAP
     * 4.)  *ptr && RO||RW == COPY (UNLESS MAP)
     */

    if ((*ptr == NULL) &&
        ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE)))
    {
        mapped = vx_true_e;
    }

    if (mapped == vx_true_e)
    {
        status = VX_ERROR_NO_RESOURCES;

        /* lock the memory */
        if(vxSemWait(&arr->memory.locks[0]) == vx_true_e)
        {
            vx_size offset = start * arr->item_size;

            *ptr = &arr->memory.ptrs[0][offset];

            vxReadFromReference(&arr->base);
            vxIncrementReference(&arr->base, VX_EXTERNAL);

            status = VX_SUCCESS;
        }
    }
    else
    {
        vx_size size = ((end - start) * arr->item_size);
        vx_uint32 a = 0u;

        if (vxAddAccessor(arr->base.context, size, usage, *ptr, &arr->base, &a) == vx_true_e)
        {
            *ptr = arr->base.context->accessors[a].ptr;

            status = VX_SUCCESS;

            if ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE))
            {
                if (vxSemWait(&arr->memory.locks[0]) == vx_false_e)
                {
                    status = VX_ERROR_NO_RESOURCES;
                }
            }

            if (status == VX_SUCCESS)
            {
                vx_size offset = start * arr->item_size;

                memcpy(*ptr, &arr->memory.ptrs[0][offset], size);

                vxReadFromReference(&arr->base);
                vxIncrementReference(&arr->base, VX_EXTERNAL);
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            vxAddLogEntry((vx_reference)arr, status, "Failed to allocate memory for COPY-ON-READ! Size="VX_FMT_SIZE"\n", size);
        }
    }

    return status;
}

vx_status vxCommitArrayRangeInt(vx_array arr, vx_size start, vx_size end, void *ptr)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    vx_bool external = vx_true_e; // assume that it was an allocated buffer

    if ((ptr == NULL) ||
        (start > end) || (end > arr->num_items))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* determine if virtual before checking for memory */
    if (arr->base.is_virtual == vx_true_e)
    {
        if (arr->base.is_accessible == vx_false_e)
        {
            /* User tried to access a "virtual" array. */
            VX_PRINT(VX_ZONE_ERROR, "Can not access a virtual array\n");
            return VX_ERROR_OPTIMIZED_AWAY;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    /* VARIABLES:
     * 1.) ZERO_AREA
     * 2.) CONSTANT - independant
     * 3.) INTERNAL - independant of area
     * 4.) EXTERNAL - dependant on area (do nothing on zero, determine on non-zero)
     * 5.) !INTERNAL && !EXTERNAL == MAPPED
     */

    {
        /* check to see if the range is zero area */
        vx_bool zero_area = (end == 0) ? vx_true_e : vx_false_e;
        vx_uint32 index = UINT32_MAX; // out of bounds, if given to remove, won't do anything
        vx_bool internal = vxFindAccessor(arr->base.context, ptr, &index);

        if (zero_area == vx_false_e)
        {
            /* this could be a write-back */
            if (internal == vx_true_e && arr->base.context->accessors[index].usage == VX_READ_ONLY)
            {
                /* this is a buffer that we allocated on behalf of the user and now they are done. Do nothing else*/
                vxRemoveAccessor(arr->base.context, index);
            }
            else
            {
                vx_uint8 *beg_ptr = arr->memory.ptrs[0];
                vx_uint8 *end_ptr = &beg_ptr[arr->item_size * arr->num_items];

                if ((beg_ptr <= (vx_uint8 *)ptr) && ((vx_uint8 *)ptr < end_ptr))
                {
                    /* the pointer in contained in the array, so it was mapped, thus
                     * there's nothing else to do. */
                    external = vx_false_e;
                }

                if (external == vx_true_e || internal == vx_true_e)
                {
                    /* the pointer was not mapped, copy. */
                    vx_size offset = start * arr->item_size;
                    vx_size len = (end - start) * arr->item_size;

                    memcpy(&beg_ptr[offset], ptr, len);

                    if (internal == vx_true_e)
                    {
                        /* a write only or read/write copy */
                        vxRemoveAccessor(arr->base.context, index);
                    }
                }

                vxWroteToReference(&arr->base);
            }

            vxSemPost(&arr->memory.locks[0]);

            status = VX_SUCCESS;
        }
        else
        {
            /* could be RO|WO|RW where they decided not to commit anything. */
            if (internal == vx_true_e) // RO
            {
                vxRemoveAccessor(arr->base.context, index);
            }
            else // RW|WO
            {
                vxSemPost(&arr->memory.locks[0]);
            }

            status = VX_SUCCESS;
        }

        vxDecrementReference(&arr->base, VX_EXTERNAL);
    }

    return status;
}

/*==============================================================================
 PUBLIC INTERFACE
 =============================================================================*/

VX_API_ENTRY vx_array VX_API_CALL vxCreateArray(vx_context context, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;

    if ((vxIsValidContext(context) == vx_true_e) &&
        (vxIsValidArrayItemType(context, item_type) == vx_true_e) &&
        (capacity > 0))
    {
        arr = (vx_array)vxCreateArrayInt(context, item_type, capacity, vx_false_e, VX_TYPE_ARRAY);

        if (arr == NULL)
        {
            arr = (vx_array)vxGetErrorObject(context, VX_ERROR_NO_MEMORY);
        }
    }
    else
    {
        arr = (vx_array)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    return arr;
}

VX_API_ENTRY vx_array VX_API_CALL vxCreateVirtualArray(vx_graph graph, vx_enum item_type, vx_size capacity)
{
    vx_array arr = NULL;
    if ((vxIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) &&
        ((vxIsValidArrayItemType(graph->base.context, item_type) == vx_true_e) || item_type == VX_TYPE_INVALID))
    {
        arr = (vx_array)vxCreateArrayInt(graph->base.context, item_type, capacity, vx_true_e, VX_TYPE_ARRAY);

        if (arr && arr->base.type == VX_TYPE_ARRAY)
        {
            arr->base.scope = (vx_reference_t *)graph;
        }
        else
        {
            arr = (vx_array)vxGetErrorObject(graph->base.context, VX_ERROR_NO_MEMORY);
        }
    }
    else
    {
        arr = (vx_array)vxGetErrorObject(graph->base.context, VX_ERROR_INVALID_PARAMETERS);
    }
    return arr;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseArray(vx_array *a)
{
    /* NULL means standard destructor */
    return vxReleaseReferenceInt((vx_reference_t **)a, VX_TYPE_ARRAY, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryArray(vx_array arr, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidArray(arr) == vx_true_e)
    {
        status = VX_SUCCESS;
        switch (attribute)
        {
            case VX_ARRAY_ATTRIBUTE_ITEMTYPE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = arr->item_type;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_ARRAY_ATTRIBUTE_NUMITEMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = arr->num_items;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_ARRAY_ATTRIBUTE_CAPACITY:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = arr->capacity;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_ARRAY_ATTRIBUTE_ITEMSIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = arr->item_size;
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

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddArrayItems(vx_array arr, vx_size count, void *ptr, vx_size stride)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidArray(arr) == vx_true_e)
    {
        status = VX_ERROR_NO_MEMORY;

        if(vxAllocateArray(arr) == vx_true_e)
        {
            status = VX_ERROR_INVALID_PARAMETERS;

            if ((count > 0) && (ptr != NULL) && (stride == 0 || stride >= arr->item_size))
            {
                status = VX_FAILURE;

                if (arr->num_items + count <= arr->capacity)
                {
                    vx_size offset = arr->num_items * arr->item_size;
                    vx_uint8 *dst_ptr = &arr->memory.ptrs[0][offset];

                    if (stride == 0 || stride == arr->item_size)
                    {
                        memcpy(dst_ptr, ptr, count * arr->item_size);
                    }
                    else
                    {
                        vx_size i;
                        for (i = 0; i < count; ++i)
                        {
                            vx_uint8 *tmp = (vx_uint8 *)ptr;
                            memcpy(&dst_ptr[i * arr->item_size], &tmp[i * stride], arr->item_size);
                        }
                    }

                    arr->num_items += count;
                    vxWroteToReference(&arr->base);

                    status = VX_SUCCESS;
                }
            }
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxTruncateArray(vx_array arr, vx_size new_num_items)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidArray(arr) == vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;

        if (new_num_items <= arr->num_items)
        {
            arr->num_items = new_num_items;
            vxWroteToReference(&arr->base);

            status = VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessArrayRange(vx_array arr, vx_size start, vx_size end, vx_size *stride, void **ptr, vx_enum usage)
{
    vx_status status = VX_FAILURE;
    /* bad references */
    if (vxIsValidArray(arr) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid array!\n");
        return VX_ERROR_INVALID_REFERENCE;
    }

    /* bad parameters */
    if (stride == NULL)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    status = vxAccessArrayRangeInt(arr, start, end, ptr, usage);

    if (status == VX_SUCCESS)
    {
        *stride = arr->item_size;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitArrayRange(vx_array arr, vx_size start, vx_size end, void *ptr)
{
    if (vxIsValidArray(arr) == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }
    return vxCommitArrayRangeInt(arr, start, end, ptr);
}
