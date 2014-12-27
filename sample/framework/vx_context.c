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

#include <vx_internal.h>

const vx_char implementation[VX_MAX_IMPLEMENTATION_NAME] = "khronos.sample";

vx_char targetModules[][VX_MAX_TARGET_NAME] = {
    "openvx-c_model",
#if defined(EXPERIMENTAL_USE_OPENCL)
    "openvx-opencl",
#endif
#if defined(EXPERIMENTAL_USE_OPENMP)
    "openvx-openmp"
#endif
};

const vx_char extensions[] =
#if defined(OPENVX_USE_TILING)
    OPENVX_KHR_TILING" "
#endif
#if defined(EXPERIMENTAL_USE_XML)
    OPENVX_KHR_XML" "
#endif
#if defined(EXPERIMENTAL_USE_OPENCL)
    OPENVX_KHR_OPENCL" "
#endif
#if defined(EXPERIMENTAL_USE_NODE_MEMORY)
    OPENVX_KHR_NODE_MEMORY" "
#endif
#if defined(EXPERIMENTAL_USE_S16)
    "vx_khr_s16 "
#endif
#if defined(EXPERIMENTAL_USE_DOT)
    OPENVX_KHR_DOT" "
#endif
#if defined(EXPERIMENTAL_USE_TARGET)
    OPENVX_EXT_TARGET" "
#endif
#if defined(EXPERIMENTAL_USE_VARIANTS)
    OPENVX_KHR_VARIANTS" "
#endif
    " ";

static vx_bool vxWorkerNode(vx_threadpool_worker_t *worker)
{
    vx_bool ret = vx_true_e;
    vx_target target = (vx_target)worker->data->v1;
    vx_node node = (vx_node)worker->data->v2;
    vx_action action = (vx_action)worker->data->v3;
    vx_uint32 p = 0;

    /* turn on access to virtual memory */
    for (p = 0u; p < node->kernel->signature.num_parameters; p++) {
        if (node->parameters[p] == NULL) continue;
        if (node->parameters[p]->is_virtual == vx_true_e) {
            node->parameters[p]->is_accessible = vx_true_e;
        }
    }

    VX_PRINT(VX_ZONE_GRAPH, "Executing %s on target %s\n", node->kernel->name, target->name);
    action = target->funcs.process(target, &node, 0, 1);
    VX_PRINT(VX_ZONE_GRAPH, "Executed %s on target %s with action %d returned\n", node->kernel->name, target->name, action);

    /* turn on access to virtual memory */
    for (p = 0u; p < node->kernel->signature.num_parameters; p++) {
        if (node->parameters[p] == NULL) continue;
        if (node->parameters[p]->is_virtual == vx_true_e) {
            // determine who is the last thread to release...
            // if this is an input, then there should be "zero" rectangles, which
            // should allow commits to work, even if the flag is lowered.
            // if this is an output, there should only be a single writer, so
            // no locks are needed. Bidirectional is not allowed to be virtual.
            node->parameters[p]->is_accessible = vx_true_e;
        }
    }

    if ((action == VX_ACTION_ABANDON) || (action == VX_ACTION_RESTART))
    {
        ret = vx_false_e;
    }
    // collect the specific results.
    worker->data->v3 = (vx_value_t)action;
    return ret;
}

static vx_value_t vxWorkerGraph(void *arg)
{
    vx_processor_t *proc = (vx_processor_t *)arg;
    VX_PRINT(VX_ZONE_CONTEXT, "Starting thread!\n");
    while (proc->running == vx_true_e)
    {
        vx_graph g = 0;
        vx_status s = VX_FAILURE;
        vx_value_set_t *data = NULL;
        if (vxReadQueue(&proc->input, &data) == vx_true_e)
        {
            g = (vx_graph)data->v1;
            // s = (vx_status)v2;
            VX_PRINT(VX_ZONE_CONTEXT, "Read graph=" VX_FMT_REF ", status=%d\n",g,s);
            s = vxProcessGraph(g);
            VX_PRINT(VX_ZONE_CONTEXT, "Writing graph=" VX_FMT_REF ", status=%d\n",g,s);
            data->v1 = (vx_value_t)g;
            data->v2 = (vx_status)s;
            if (vxWriteQueue(&proc->output, data) == vx_false_e)
                VX_PRINT(VX_ZONE_ERROR, "Failed to write graph=" VX_FMT_REF " status=%d\n", g, s);
        }
    }
    VX_PRINT(VX_ZONE_CONTEXT,"Stopping thread!\n");
    return 0;
}

VX_INT_API vx_bool vxIsValidType(vx_enum type)
{
    vx_bool ret = vx_false_e;
    if (type <= VX_TYPE_INVALID)
    {
        ret = vx_false_e;
    }
    else if (VX_TYPE_IS_SCALAR(type)) /* some scalar */
    {
        ret = vx_true_e;
    }
    else if (VX_TYPE_IS_STRUCT(type)) /* some struct */
    {
        ret = vx_true_e;
    }
    else if (VX_TYPE_IS_OBJECT(type)) /* some object */
    {
        ret = vx_true_e;
    }
#ifdef OPENVX_KHR_XML
    else if (type == VX_TYPE_IMPORT) /* import type extension */
    {
        ret = vx_true_e;
    }
#endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Type 0x%08x is invalid!\n");
    }
    return ret; /* otherwise, not a valid type */
}

VX_INT_API vx_bool vxIsValidImport(vx_enum type)
{
    vx_bool ret = vx_false_e;
    switch(type)
    {
        case VX_IMPORT_TYPE_HOST:
            ret = vx_true_e;
            break;
        case VX_IMPORT_TYPE_NONE:
        default:
            ret = vx_false_e;
            break;
    }
    return ret;
}

VX_INT_API vx_bool vxIsValidContext(vx_context context)
{
    vx_bool ret = vx_false_e;
    if ((context != NULL) &&
        (context->base.magic == VX_MAGIC) &&
        (context->base.type == VX_TYPE_CONTEXT) &&
        (context->base.context == NULL))
    {
        ret = vx_true_e; /* this is the top level context */
    }
    if (ret == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "%p is not a valid context!\n", context);
    }
    return ret;
}

VX_INT_API vx_bool vxIsValidBorderMode(vx_enum mode)
{
    vx_bool ret = vx_true_e;
    switch (mode)
    {
        case VX_BORDER_MODE_UNDEFINED:
        case VX_BORDER_MODE_CONSTANT:
        case VX_BORDER_MODE_REPLICATE:
            break;
        default:
            ret = vx_false_e;
            break;
    }
    return ret;
}

VX_INT_API vx_bool vxAddAccessor(vx_context context,
                      vx_size size,
                      vx_enum usage,
                      void *ptr,
                      vx_reference ref,
                      vx_uint32 *pIndex)
{
    vx_uint32 a;
    vx_bool worked = vx_false_e;
    for (a = 0u; a < dimof(context->accessors); a++)
    {
        if (context->accessors[a].used == vx_false_e)
        {
            VX_PRINT(VX_ZONE_CONTEXT, "Found open accessors[%u]\n", a);
            if (size > 0ul && ptr == NULL)
            {
                context->accessors[a].ptr = malloc(size);
                if (context->accessors[a].ptr == NULL)
                    return vx_false_e;
                context->accessors[a].allocated = vx_true_e;
            }
            else
            {
                context->accessors[a].ptr = ptr;
                context->accessors[a].allocated = vx_false_e;
            }
            context->accessors[a].usage = usage;
            context->accessors[a].ref = ref;
            context->accessors[a].used = vx_true_e;
            if (pIndex) *pIndex = a;
            worked = vx_true_e;
            break;
        }
    }
    return worked;
}

VX_INT_API vx_bool vxFindAccessor(vx_context context, void *ptr, vx_uint32 *pIndex)
{
    vx_uint32 a;
    vx_bool worked = vx_false_e;
    for (a = 0u; a < dimof(context->accessors); a++)
    {
        if (context->accessors[a].used == vx_true_e)
        {
            if (context->accessors[a].ptr == ptr)
            {
                VX_PRINT(VX_ZONE_CONTEXT, "Found accessors[%u] for %p\n", a, ptr);
                worked = vx_true_e;
                if (pIndex) *pIndex = a;
                break;
            }
        }
    }
    return worked;
}

VX_INT_API void vxRemoveAccessor(vx_context context, vx_uint32 index)
{
    if (index < dimof(context->accessors))
    {
        if (context->accessors[index].allocated == vx_true_e)
        {
            free(context->accessors[index].ptr);
        }
        memset(&context->accessors[index], 0, sizeof(vx_external_t));
        VX_PRINT(VX_ZONE_CONTEXT, "Removed accessors[%u]\n", index);
    }
}

/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/

static vx_context_t *single_context = NULL;
static vx_sem_t context_lock;
static vx_sem_t global_lock;

VX_API_ENTRY vx_context VX_API_CALL vxCreateContext()
{
    vx_context context = NULL;

    if (single_context == NULL)
    {
        vxCreateSem(&context_lock, 1);
        vxCreateSem(&global_lock, 1);
    }

    vxSemWait(&context_lock);
    if (single_context == NULL)
    {
        /* read the variables for debugging flags */
        vx_set_debug_zone_from_env();

        context = VX_CALLOC(vx_context_t); /* \todo get from allocator? */
        if (context)
        {
            vx_uint32 p = 0u, p2 = 0u, t = 0u;
            context->p_global_lock = &global_lock;
            context->imm_border.mode = VX_BORDER_MODE_UNDEFINED;
            vxInitReference(&context->base, NULL, VX_TYPE_CONTEXT, NULL);
            vxIncrementReference(&context->base, VX_EXTERNAL);
            context->workers = vxCreateThreadpool(VX_INT_HOST_CORES,
                                                  VX_INT_MAX_REF, /* very deep queues! */
                                                  sizeof(vx_work_t),
                                                  vxWorkerNode,
                                                  context);
            vxCreateConstErrors(context);

            /* load all targets */
            for (t = 0u; t < dimof(targetModules); t++)
            {
                if (vxLoadTarget(context, targetModules[t]) == VX_SUCCESS)
                {
                    context->num_targets++;
                }
            }

            if (context->num_targets == 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "No targets loaded!\n");
                free(context);
                vxSemPost(&context_lock);
                return 0;
            }

            /* initialize all targets */
            for (t = 0u; t < context->num_targets; t++)
            {
                if (context->targets[t].module.handle)
                {
                    /* call the init function */
                    if (context->targets[t].funcs.init(&context->targets[t]) != VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_WARNING, "Target %s failed to initialize!\n", context->targets[t].name);
                        /* unload this module */
                        vxUnloadTarget(context, t, vx_true_e);
                        break;
                    }
                    else
                    {
                        context->targets[t].enabled = vx_true_e;
                    }
                }
            }

            /* assign the targets by priority into the list */
            p2 = 0u;
            for (p = 0u; p < VX_TARGET_PRIORITY_MAX; p++)
            {
                for (t = 0u; t < context->num_targets; t++)
                {
                    vx_target_t * target = &context->targets[t];
                    if (p == target->priority)
                    {
                        context->priority_targets[p2] = t;
                        p2++;
                    }
                }
            }
            /* print out the priority list */
            for (t = 0u; t < context->num_targets; t++)
            {
                vx_target_t *target = &context->targets[context->priority_targets[t]];
                if (target->enabled == vx_true_e)
                {
                    VX_PRINT(VX_ZONE_TARGET, "target[%u]: %s\n",
                                target->priority,
                                target->name);
                }
            }

            // create the internal thread which processes graphs for asynchronous mode.
            vxInitQueue(&context->proc.input);
            vxInitQueue(&context->proc.output);
            context->proc.running = vx_true_e;
            context->proc.thread = vxCreateThread(vxWorkerGraph, &context->proc);
            single_context = context;
        }
    }
    else
    {
        context = single_context;
        vxIncrementReference(&context->base, VX_EXTERNAL);
    }
    vxSemPost(&context_lock);
    return (vx_context)context;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseContext(vx_context *c)
{
    vx_status status = VX_SUCCESS;
    vx_context context = (c?*c:0);
    vx_uint32 r,m,a;
    vx_uint32 t;

    if (c) *c = 0;
    vxSemWait(&context_lock);
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (vxDecrementReference(&context->base, VX_EXTERNAL) == 0)
        {
            vxDestroyThreadpool(&context->workers);
            context->proc.running = vx_false_e;
            vxPopQueue(&context->proc.input);
            vxJoinThread(context->proc.thread, NULL);
            vxDeinitQueue(&context->proc.output);
            vxDeinitQueue(&context->proc.input);

            /* Deregister any log callbacks if there is any registered */
            vxRegisterLogCallback(context, NULL, vx_false_e);

            /*! \internal Garbage Collect All References */
            /* Details:
             *   1. This loop will warn of references which have not been released by the user.
             *   2. It will close all internally opened error references.
             *   3. It will close the external references, which in turn will internally
             *      close any internally dependent references that they reference, assuming the
             *      reference counting has been done properly in the framework.
             *   4. This garbage collection must be done before the targets are released since some of
             *      these external references may have internal references to target kernels.
             */
            for (r = 0; r < VX_INT_MAX_REF; r++)
            {
                vx_reference_t *ref = context->reftable[r];

                /* Warnings should only come when users have not released all external references */
                if (ref && ref->external_count > 0) {
                    VX_PRINT(VX_ZONE_WARNING,"Stale reference "VX_FMT_REF" of type %08x at external count %u, internal count %u\n",
                             ref, ref->type, ref->external_count, ref->internal_count);
                }

                /* These were internally opened during creation, so should internally close ERRORs */
                if(ref && ref->type == VX_TYPE_ERROR) {
                    vxReleaseReferenceInt(&ref, ref->type, VX_INTERNAL, NULL);
                }

                /* Warning above so user can fix release external objects, but close here anyway */
                while (ref && ref->external_count > 1) {
                    vxDecrementReference(ref, VX_EXTERNAL);
                }
                if (ref && ref->external_count > 0) {
                    vxReleaseReferenceInt(&ref, ref->type, VX_EXTERNAL, NULL);
                }

            }

            for (m = 0; m < context->num_modules; m++)
            {
                if (context->modules[m].handle)
                {
                    vxUnloadModule(context->modules[m].handle);
                    memset(context->modules[m].name, 0, sizeof(context->modules[m].name));
                    context->modules[m].handle = VX_MODULE_INIT;
                }
            }

            /* de-initialize and unload each target */
            for (t = 0u; t < context->num_targets; t++)
            {
                if (context->targets[t].enabled == vx_true_e)
                {
                    context->targets[t].funcs.deinit(&context->targets[t]);
                    vxUnloadTarget(context, t, vx_true_e);
                    context->targets[t].enabled = vx_false_e;
                }
            }

            /* Remove all outstanding accessors. */
            for (a = 0; a < dimof(context->accessors); ++a)
                if (context->accessors[a].used)
                    vxRemoveAccessor(context, a);

            /* By now, all external and internal references should be removed */
            for (r = 0; r < VX_INT_MAX_REF; r++)
            {
                if(context->reftable[r])
                    VX_PRINT(VX_ZONE_ERROR,"Reference %d not removed\n", r);
            }

            /*! \internal wipe away the context memory first */
            /* Normally destroy sem is part of release reference, but can't for context */
            vxDestroySem(&((vx_reference )context)->lock);
            memset(context, 0, sizeof(vx_context_t));
            free((void *)context);
            vxDestroySem(&global_lock);
            vxSemPost(&context_lock);
            vxDestroySem(&context_lock);
            single_context = NULL;
            return status;
        }
        else
        {
            VX_PRINT(VX_ZONE_WARNING, "Context still has %u holders\n", vxTotalReferenceCount(&context->base));
        }
    } else {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    vxSemPost(&context_lock);
    return status;
}

VX_API_ENTRY vx_context VX_API_CALL vxGetContext(vx_reference reference)
{
    vx_context context = NULL;
    if (vxIsValidReference(reference) == vx_true_e)
    {
        context = reference->context;
    }
    else if (vxIsValidContext((vx_context)reference) == vx_true_e)
    {
        context = (vx_context)reference;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "%p is not a valid reference\n", reference);
        VX_BACKTRACE(VX_ZONE_ERROR);
    }
    return context;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetContextAttribute(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidContext(context) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute) {
            case VX_CONTEXT_ATTRIBUTE_IMMEDIATE_BORDER_MODE:
                if (VX_CHECK_PARAM(ptr, size, vx_border_mode_t, 0x3))
                {
                    vx_border_mode_t *config = (vx_border_mode_t *)ptr;
                    if (vxIsValidBorderMode(config->mode) == vx_false_e)
                        status = VX_ERROR_INVALID_VALUE;
                    else
                    {
                        context->imm_border = *config;
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
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryContext(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidContext(context) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case VX_CONTEXT_ATTRIBUTE_VENDOR_ID:
                if (VX_CHECK_PARAM(ptr, size, vx_uint16, 0x1))
                {
                    *(vx_uint16 *)ptr = VX_ID_KHRONOS;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_VERSION:
                if (VX_CHECK_PARAM(ptr, size, vx_uint16, 0x1))
                {
                    *(vx_uint16 *)ptr = (vx_uint16)VX_VERSION;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_MODULES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_modules;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_REFERENCES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_references;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#if defined(EXPERIMENTAL_USE_TARGET)
            case VX_CONTEXT_ATTRIBUTE_TARGETS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_targets;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#endif
            case VX_CONTEXT_ATTRIBUTE_IMPLEMENTATION:
                if (size <= VX_MAX_IMPLEMENTATION_NAME && ptr)
                {
                    strncpy(ptr, implementation, VX_MAX_IMPLEMENTATION_NAME);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_EXTENSIONS_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = sizeof(extensions);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_EXTENSIONS:
                if (size <= sizeof(extensions) && ptr)
                {
                    strncpy(ptr, extensions, sizeof(extensions));
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_CONVOLUTION_MAXIMUM_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = VX_INT_MAX_CONVOLUTION_DIM;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_OPTICAL_FLOW_WINDOW_MAXIMUM_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = VX_OPTICALFLOWPYRLK_MAX_DIM;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_IMMEDIATE_BORDER_MODE:
                if (VX_CHECK_PARAM(ptr, size, vx_border_mode_t, 0x3))
                {
                    *(vx_border_mode_t *)ptr = context->imm_border;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNELS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = context->num_unique_kernels;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNEL_TABLE:
                if ((size == (context->num_unique_kernels * sizeof(vx_kernel_info_t))) &&
                    (ptr != NULL)) {
                    vx_uint32 k = 0u, t = 0u, k2 = 0u, numk = 0u;
                    vx_kernel_info_t *table = (vx_kernel_info_t *)ptr;
                    for (t = 0; t < context->num_targets; t++) {
                        for (k = 0u; k < context->targets[t].num_kernels; k++) {
                            vx_bool found = vx_false_e;
                            VX_PRINT(VX_ZONE_INFO, "Checking uniqueness of %s (%d)\n", context->targets[t].kernels[k].name, context->targets[t].kernels[k].enumeration);
                            for (k2 = 0u; k2 < numk; k2++) {
                                if (table[k2].enumeration == context->targets[t].kernels[k].enumeration) {
                                    found = vx_true_e;
                                    break;
                                }
                            }
                            if (found == vx_false_e) {
                                VX_PRINT(VX_ZONE_INFO, "Kernel %s is unique\n", context->targets[t].kernels[k].name);
                                table[numk].enumeration = context->targets[t].kernels[k].enumeration;
#if defined(EXPERIMENTAL_USE_TARGET) || defined(EXPERIMENTAL_USE_VARIANT)
                                // get the central string out
                                {
                                    vx_uint32 c = 0;
                                    strncpy(table[numk].name, context->targets[t].kernels[k].name, VX_MAX_KERNEL_NAME);
                                    for (c = 0; table[numk].name[c] != '\0'; c++) {
                                        if (table[numk].name[c] == ';') {
                                            table[numk].name[c] = '\0';
                                            break;
                                        }
                                    }
                                }
#else
                                strncpy(table[numk].name, context->targets[t].kernels[k].name, VX_MAX_KERNEL_NAME);
#endif
                                numk++;
                            }
                        }
                    }
                } else {
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

VX_API_ENTRY vx_status VX_API_CALL vxHint(vx_context context, vx_reference reference, vx_enum hint) {
    vx_status status = VX_SUCCESS;
    if (vxIsValidContext(context) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidReference(reference) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;
    switch (hint)
    {
        /*! \todo add hints to the sample implementation */
        case VX_HINT_SERIALIZE:
            /* toggle */
            if (vxIsValidSpecificReference(reference, VX_TYPE_GRAPH) == vx_true_e) {
                vx_graph graph = (vx_graph)reference;
                graph->should_serialize = (graph->should_serialize ? vx_false_e : vx_true_e);
            }
            else{
                status = VX_ERROR_INVALID_TYPE;
            }
            break;
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxDirective(vx_context context, vx_reference reference, vx_enum directive) {
    vx_status status = VX_SUCCESS;
    if (vxIsValidContext(context) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidReference(reference) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;
    switch (directive)
    {
        case VX_DIRECTIVE_DISABLE_LOGGING:
            context->log_enabled = vx_false_e;
            break;
        case VX_DIRECTIVE_ENABLE_LOGGING:
            context->log_enabled = vx_true_e;
            break;
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }
    return status;
}

VX_API_ENTRY vx_enum VX_API_CALL vxRegisterUserStruct(vx_context context, vx_size size)
{
    vx_enum type = VX_TYPE_INVALID;
    vx_uint32 i = 0;

    if ((vxIsValidContext(context) == vx_true_e) &&
        (size != 0))
    {
        for (i = 0; i < VX_INT_MAX_USER_STRUCTS; ++i)
        {
            if (context->user_structs[i].type == VX_TYPE_INVALID)
            {
                context->user_structs[i].type = VX_TYPE_USER_STRUCT_START + i;
                context->user_structs[i].size = size;
                type = context->user_structs[i].type;
                break;
            }
        }
    }
    return type;
}
