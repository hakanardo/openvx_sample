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


void vxPrintTarget(vx_target target, vx_uint32 index)
{
    if (target)
    {
        vxPrintReference(&target->base);
        VX_PRINT(VX_ZONE_TARGET, "Target[%u]=>%s\n", index, target->name);
    }
}

vx_status vxUnloadTarget(vx_context_t *context, vx_uint32 index, vx_bool unload_module)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_target target = &context->targets[index];
    vx_target *targ = &target;
    if (index < VX_INT_MAX_NUM_TARGETS)
    {
        memset(&context->targets[index].funcs, 0xFE, sizeof(vx_target_funcs_t));
        if (vxDecrementReference(&context->targets[index].base, VX_INTERNAL) == 0)
        {
            /* The vxReleaseReferenceInt() below errors out if the internal index is 0 */
            vxIncrementReference(&context->targets[index].base, VX_INTERNAL);
            if (unload_module)
            {
                vxUnloadModule(context->targets[index].module.handle);
                context->targets[index].module.handle = VX_MODULE_INIT;
            }

            memset(context->targets[index].module.name, 0, sizeof(context->targets[index].module.name));
            vxReleaseReferenceInt((vx_reference *)targ, VX_TYPE_TARGET, VX_INTERNAL, NULL);
        }
        status = VX_SUCCESS;
    }
    return status;
}

vx_status vxLoadTarget(vx_context context, vx_char *name)
{
    vx_status status = VX_FAILURE;
    vx_uint32 index = 0u;

    for (index = 0; index < VX_INT_MAX_NUM_TARGETS; index++)
    {
        if (context->targets[index].module.handle == VX_MODULE_INIT)
        {
            break;
        }
    }
    if (index < VX_INT_MAX_NUM_TARGETS)
    {
        vx_char module[VX_INT_MAX_PATH];

#if defined(WIN32)
        _snprintf(module, VX_INT_MAX_PATH, VX_MODULE_NAME("%s"), (name?name:"openvx-target"));
        module[VX_INT_MAX_PATH-1] = 0; // for MSVC which is not C99 compliant
#else
        snprintf(module, VX_INT_MAX_PATH, VX_MODULE_NAME("%s"), (name?name:"openvx-target"));
#endif
        context->targets[index].module.handle = vxLoadModule(module);
        if (context->targets[index].module.handle)
        {
            vxInitReference(&context->targets[index].base, context, VX_TYPE_TARGET, &context->base);
            vxIncrementReference(&context->targets[index].base, VX_INTERNAL);
            vxAddReference(context, &context->targets[index].base);

            context->targets[index].funcs.init     = (vx_target_init_f)    vxGetSymbol(context->targets[index].module.handle, "vxTargetInit");
            context->targets[index].funcs.deinit   = (vx_target_deinit_f)  vxGetSymbol(context->targets[index].module.handle, "vxTargetDeinit");
            context->targets[index].funcs.supports = (vx_target_supports_f)vxGetSymbol(context->targets[index].module.handle, "vxTargetSupports");
            context->targets[index].funcs.process  = (vx_target_process_f) vxGetSymbol(context->targets[index].module.handle, "vxTargetProcess");
            context->targets[index].funcs.verify   = (vx_target_verify_f)  vxGetSymbol(context->targets[index].module.handle, "vxTargetVerify");
            context->targets[index].funcs.addkernel= (vx_target_addkernel_f)vxGetSymbol(context->targets[index].module.handle, "vxTargetAddKernel");
#ifdef OPENVX_KHR_TILING
            context->targets[index].funcs.addtilingkernel = (vx_target_addtilingkernel_f)vxGetSymbol(context->targets[index].module.handle, "vxTargetAddTilingKernel");
#endif
            if (context->targets[index].funcs.init &&
                context->targets[index].funcs.deinit &&
                context->targets[index].funcs.supports &&
                context->targets[index].funcs.process &&
                context->targets[index].funcs.verify &&
                context->targets[index].funcs.addkernel)
                /* tiling kernel function can be NULL */
            {
                VX_PRINT(VX_ZONE_TARGET, "Loaded target %s\n", module);
                status = VX_SUCCESS;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to load target %s due to missing symbols!\n", module);
                vxUnloadTarget(context, index, vx_true_e);
                status = VX_ERROR_NO_RESOURCES;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_WARNING, "Failed to load target %s\n", module);
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "No free targets remain!\n");
    }
    return status;
}

vx_uint32 vxFindTargetIndex(vx_target target)
{
    vx_uint32 t = 0u;
    for (t = 0u; t < target->base.context->num_kernels; t++)
    {
        if (target == &target->base.context->targets[t])
        {
            break;
        }
    }
    return t;
}

vx_status vxInitializeTarget(vx_target target,
                             vx_kernel_description_t *kernels[],
                             vx_uint32 numkernels)
{
    vx_uint32 k = 0u;
    vx_status status = VX_FAILURE;
    for (k = 0u; k < numkernels; k++)
    {
        status = vxInitializeKernel(target->base.context, &target->kernels[k],
                                   kernels[k]->enumeration,
                                   kernels[k]->function,
                                   kernels[k]->name,
                                   kernels[k]->parameters,
                                   kernels[k]->numParams,
                                   kernels[k]->input_validate,
                                   kernels[k]->output_validate,
                                   kernels[k]->initialize,
                                   kernels[k]->deinitialize);
        VX_PRINT(VX_ZONE_KERNEL, "Initialized Kernel %s, %d\n", kernels[k]->name, status);
        if (status == VX_SUCCESS) {
            if (vxIsKernelUnique(&target->kernels[k]) == vx_true_e) {
                VX_PRINT(VX_ZONE_KERNEL, "Kernel %s is unique\n", kernels[k]->name);
                target->base.context->num_unique_kernels++;
            }
            target->base.context->num_kernels++;
            target->num_kernels++;
        }
    }
    return status;
}

vx_status vxDeinitializeTarget(vx_target target)
{
    vx_uint32 k = 0u;
    vx_status status = VX_SUCCESS;
    vx_kernel kernel = NULL;
    for (k = 0u; k < target->num_kernels; k++)
    {
        kernel = &(target->kernels[k]);
        if ((kernel->enabled != vx_false_e) ||
            (kernel->enumeration != VX_KERNEL_INVALID))
        {
            kernel->enabled = vx_false_e;
            if (vxIsKernelUnique(&target->kernels[k]) == vx_true_e) {
                target->base.context->num_unique_kernels--;
            }
            if( vxDeinitializeKernel(&kernel) != VX_SUCCESS ) {
                status = VX_FAILURE;
            }
        }
    }
    target->base.context->num_kernels-=target->num_kernels;
    target->num_kernels = 0;
    return status;
}


/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/

#if defined(EXPERIMENTAL_USE_TARGET)

VX_API_ENTRY vx_target VX_API_CALL vxGetTargetByIndex(vx_context context, vx_uint32 index)
{
    vx_target target = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (index < context->num_targets)
        {
            target = (vx_target_t*)&context->targets[index];
            vxIncrementReference(&target->base, VX_EXTERNAL);
            vxPrintTarget(target, index);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect index value\n");
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Incorrect index value\n");
            target = (vx_target_t *)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid context!\n");
    }
    return (vx_target)target;
}

VX_API_ENTRY vx_target VX_API_CALL vxGetTargetByName(vx_context context, const vx_char *name)
{
    vx_target target = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        vx_uint32 index = 0;
        for (index = 0; index < context->num_targets; index++)
        {
            if (strncmp(name, context->targets[index].name, VX_MAX_TARGET_NAME) == 0)
            {
                target = (vx_target_t *)&context->targets[index];
                vxIncrementReference(&target->base, VX_EXTERNAL);
                vxPrintTarget(target, index);
                break;
            }
        }
    }
    return (vx_target)target;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseTarget(vx_target *t)
{
    vx_status status = VX_SUCCESS;
    vx_target target = (t?*t:0);
    if (vxIsValidSpecificReference((vx_reference_t *)target, VX_TYPE_TARGET) == vx_true_e)
    {
        vxDecrementReference(&target->base, VX_EXTERNAL);
    } else {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    if (t) *t = 0;
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryTarget(vx_target target, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference((vx_reference_t *)target, VX_TYPE_TARGET) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_TARGET_ATTRIBUTE_INDEX:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    vx_uint32 t = vxFindTargetIndex(target);
                    if (t == target->base.context->num_targets)
                    {
                        status = VX_ERROR_INVALID_PARAMETERS;
                    }
                    else
                    {
                        *(vx_uint32 *)ptr = t;
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_TARGET_ATTRIBUTE_NAME:
                if ((size <= VX_MAX_TARGET_NAME) && (ptr != NULL))
                {
                    strncpy(ptr, target->name, size);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_TARGET_ATTRIBUTE_NUMKERNELS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = target->num_kernels;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_TARGET_ATTRIBUTE_KERNELTABLE:
                if ((size == target->num_kernels * sizeof(vx_kernel_info_t)) &&
                    (ptr != NULL))
                {
                    vx_uint32 k;
                    vx_kernel_info_t *table = (vx_kernel_info_t *)ptr;
                    VX_PRINT(VX_ZONE_TARGET, "There are %u kernels\n", target->num_kernels);
                    for (k = 0; k < target->num_kernels; k++)
                    {
                        table[k].enumeration = target->kernels[k].enumeration;
                        strncpy(table[k].name, target->kernels[k].name, VX_MAX_KERNEL_NAME);
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
        status = VX_ERROR_INVALID_REFERENCE;
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeAffinity(vx_node node, vx_target target)
{
    vx_status status = VX_ERROR_NOT_SUPPORTED;
    if (vxIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e &&
        vxIsValidSpecificReference(&target->base, VX_TYPE_TARGET) == vx_true_e)
    {
        /* if this target has this kernel, then execute it */
        vx_uint32 k = 0;
        for (k = 0; k < target->num_kernels; k++)
        {
            if (node->kernel->enumeration == target->kernels[k].enumeration)
            {
                node->affinity = vxFindTargetIndex(target);
                VX_PRINT(VX_ZONE_TARGET, "Assigned Node %s to Target %s\n",
                         node->kernel->name,
                         node->base.context->targets[node->affinity].name);
                status = VX_SUCCESS;
                break;
            }
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

#endif

