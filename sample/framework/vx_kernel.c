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

void vxPrintKernel(vx_kernel kernel)
{
    VX_PRINT(VX_ZONE_KERNEL, "kernel[%u] enabled?=%s %s \n",
            kernel->enumeration,
            (kernel->enabled?"TRUE":"FALSE"),
            kernel->name);
}

vx_bool vxIsKernelUnique(vx_kernel kernel) {
    vx_uint32 t = 0u, k = 0u;
    vx_context context = kernel->base.context;
    vx_bool unique = vx_true_e;
    for (t = 0u; t < context->num_targets; t++) {
        for (k = 0u; k < context->targets[t].num_kernels; k++) {
            if (context->targets[t].kernels[k].enabled && context->targets[t].kernels[k].enumeration == kernel->enumeration) {
                unique = vx_false_e;
                break;
            }
        }
    }
    if (unique == vx_true_e) {
        VX_PRINT(VX_ZONE_KERNEL, "Kernel %s (%x) is unique!\n", kernel->name, kernel->enumeration);
    }
    return unique;
}

vx_kernel_t *vxAllocateKernel(vx_context context,
                              vx_enum kenum,
                              vx_kernel_f function,
                              vx_char name[VX_MAX_KERNEL_NAME],
                              vx_param_description_t *parameters,
                              vx_uint32 numParams)
{
    vx_kernel kernel = (vx_kernel)vxCreateReference(context, VX_TYPE_KERNEL, VX_INTERNAL, &context->base);
    if (kernel && kernel->base.type == VX_TYPE_KERNEL)
    {
        /* setup the kernel meta-data */
        strncpy(kernel->name, name, VX_MAX_KERNEL_NAME);
        kernel->enumeration = kenum;
        kernel->function = function;
        kernel->signature.num_parameters = numParams;
        kernel->attributes.borders.mode = VX_BORDER_MODE_UNDEFINED;
        if (kernel->signature.num_parameters <= VX_INT_MAX_PARAMS)
        {
            vx_uint32 p = 0;
            if (parameters != NULL)
            {
                for (p = 0; p < numParams; p++)
                {
                    kernel->signature.directions[p] = parameters[p].direction;
                    kernel->signature.types[p] = parameters[p].data_type;
                }
            }
        }
        else
        {
            free(kernel);
            kernel = NULL;
        }
    }
    return kernel;
}

vx_status vxInitializeKernel(vx_context context,
                             vx_kernel kernel,
                             vx_enum kenum,
                             vx_kernel_f function,
                             vx_char name[VX_MAX_KERNEL_NAME],
                             vx_param_description_t *parameters,
                             vx_uint32 numParams,
                             vx_kernel_input_validate_f in_validator,
                             vx_kernel_output_validate_f out_validator,
                             vx_kernel_initialize_f initialize,
                             vx_kernel_deinitialize_f deinitialize)
{
    if (kernel)
    {
        /* how to get rid of this? */
        vxInitReference(&kernel->base, context, VX_TYPE_KERNEL, &context->base);
        vxAddReference(context, &kernel->base);
        vxIncrementReference(&kernel->base, VX_INTERNAL);

        // setup the kernel meta-data
        strncpy(kernel->name, name, VX_MAX_KERNEL_NAME);
        kernel->enumeration = kenum;
        kernel->function = function;
        kernel->signature.num_parameters = numParams;
        kernel->validate_input = in_validator;
        kernel->validate_output = out_validator;
        kernel->initialize = initialize;
        kernel->deinitialize = deinitialize;
        kernel->attributes.borders.mode = VX_BORDER_MODE_UNDEFINED;
        kernel->attributes.borders.constant_value = 0;
        if (kernel->signature.num_parameters <= VX_INT_MAX_PARAMS)
        {
            vx_uint32 p = 0;
            if (parameters != NULL)
            {
                for (p = 0; p < numParams; p++)
                {
                    kernel->signature.directions[p] = parameters[p].direction;
                    kernel->signature.types[p] = parameters[p].data_type;
                    kernel->signature.states[p] = parameters[p].state;
                }
                kernel->enabled = vx_true_e;
            }
        }
        return VX_SUCCESS;
    } else {
        return VX_FAILURE;
    }
}

vx_status vxDeinitializeKernel(vx_kernel *kernel)
{
    vx_status status = VX_SUCCESS;
    vx_kernel kern = (kernel?*kernel:0);
    if (vxIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        VX_PRINT(VX_ZONE_KERNEL, "Releasing kernel "VX_FMT_REF"\n", (void *)kern);
        vxReleaseReferenceInt((vx_reference *)kernel, VX_TYPE_KERNEL, VX_INTERNAL, NULL);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Invalid Reference!\n");
    }
    return status;
}

static vx_size strnindex(vx_char *str, vx_char c, vx_size limit)
{
    vx_size index = 0;
    while (index < limit && *str != c)
    {
        if(!*str)
        {
            index = limit;
            break;
        }
        str++;
        index++;
    }
    return index;
}

/******************************************************************************/
/* PUBLIC FUNCTIONS */
/******************************************************************************/

VX_API_ENTRY vx_status VX_API_CALL vxLoadKernels(vx_context context, vx_char *name)
{
    vx_status status = VX_FAILURE;
    vx_char module[VX_INT_MAX_PATH];
    vx_uint32 m = 0;
    vx_publish_kernels_f publish = NULL;

    sprintf(module, VX_MODULE_NAME("%s"), (name?name:"openvx-ext"));

    if (vxIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Context is invalid!\n");
        return VX_ERROR_INVALID_REFERENCE;
    }

    for (m = 0; m < context->num_modules; m++)
    {
        if (context->modules[m].handle != NULL && strncmp(name, context->modules[m].name, VX_INT_MAX_PATH) == 0)
            return VX_SUCCESS;
    }

    for (m = 0; m < VX_INT_MAX_MODULES; m++)
    {
        if (context->modules[m].handle == NULL)
        {
            context->modules[m].handle = vxLoadModule(module);
            if (context->modules[m].handle)
            {
                vx_symbol_t sym = vxGetSymbol(context->modules[m].handle, "vxPublishKernels");
                publish = (vx_publish_kernels_f)sym;
                if (publish == NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to load symbol vxPublishKernels\n");
                    status = VX_ERROR_INVALID_MODULE;
                    vxUnloadModule(context->modules[m].handle);
                    context->modules[m].handle = NULL;
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO, "Calling %s publish function\n", module);
                    status = publish((vx_context)context);
                    if (status != VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to publish kernels in module\n");
                        vxUnloadModule(context->modules[m].handle);
                        context->modules[m].handle = NULL;
                    }
                    else
                    {
                        strncpy(context->modules[m].name, name, VX_INT_MAX_PATH);
                        context->num_modules++;
                    }
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to find module %s in libraries path\n", module);
            }
            break;
        }
        else
        {
            VX_PRINT(VX_ZONE_CONTEXT, "module[%u] is used\n", m);
        }
    }
    if (status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to load module %s; error %d\n", module, status);
    }
    else
    {
        for (m = 0; m < context->num_modules; m++)
        {
            VX_PRINT(VX_ZONE_INFO, "Module: %s\n", context->modules[m].name);
        }
    }
    return status;
}

static vx_size strncount(vx_char string[], vx_size size, vx_char c)
{
    vx_size i = 0ul, count = 0ul;
    while (string[i] != '\0' && i < size)
        if (string[i++] == c)
            count++;
    return count;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByName(vx_context context, vx_char string[VX_MAX_KERNEL_NAME])
{
    vx_kernel_t *kern = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        vx_uint32 k = 0u, t = 0u;
        vx_size colons = strncount(string, VX_MAX_KERNEL_NAME, ':');
        vx_char targetName[VX_MAX_TARGET_NAME] = "default";
        vx_char kernelName[VX_MAX_KERNEL_NAME];
#if defined(EXPERIMENTAL_USE_VARIANTS)
        vx_char variantName[VX_MAX_VARIANT_NAME] = "default";
#if defined(EXPERIMENTAL_USE_TARGET)
        vx_char defaultTargets[][VX_MAX_TARGET_NAME] = {
            "default",
            "power",
            "performance",
            "memory",
            "bandwidth",
        };
#endif
#endif
#if defined(_WIN32)
        vx_char *nameBuffer = _strdup(string);
#else
        vx_char *nameBuffer = strdup(string);
#endif

        if (colons == 0) {
            strncpy(kernelName, string, VX_MAX_KERNEL_NAME);
        }
        else if (colons == 1)
        {
#if defined(EXPERIMENTAL_USE_TARGET) || defined(EXPERIMENTAL_USE_VARIANTS)
            /* could be either target:kernel or kernel:variant" */
            vx_char *front = strtok(nameBuffer, ":");
            vx_char *back = strtok(NULL, ":");
#if defined(EXPERIMENTAL_USE_TARGET) && defined(EXPERIMENTAL_USE_VARIANTS)
            vx_bool isTarget = vx_false_e;
            /* does front match any targets? */
            for (t = 0u; t < context->num_targets; t++)
            {
                if (strncmp(front, context->targets[t].name, VX_MAX_TARGET_NAME) == 0)
                {
                    isTarget = vx_true_e;
                    break;
                }
            }
            if (isTarget == vx_false_e)
            {
                for (t = 0u; t < dimof(defaultTargets); t++)
                {
                    if (strncmp(front, defaultTargets[t], VX_MAX_TARGET_NAME) == 0)
                    {
                        isTarget = vx_true_e;
                        break;
                    }
                }
            }
            if (isTarget == vx_true_e)
            {
                strncpy(targetName, front, VX_MAX_TARGET_NAME);
                strncpy(kernelName, back, VX_MAX_KERNEL_NAME);
            }
            else
            {
                strncpy(kernelName, front, VX_MAX_KERNEL_NAME);
                strncpy(variantName, back, VX_MAX_VARIANT_NAME);
            }
#elif defined(EXPERIMENTAL_USE_TARGET)
            strncpy(targetName, front, VX_MAX_TARGET_NAME);
            strncpy(kernelName, back, VX_MAX_KERNEL_NAME);
#elif defined(EXPERIMENTAL_USE_VARIANTS)
            strncpy(kernelName, front, VX_MAX_KERNEL_NAME);
            strncpy(variantName, back, VX_MAX_VARIANT_NAME);
#endif
#else   /* defined(EXPERIMENTAL_USE_TARGET) || defined(EXPERIMENTAL_USE_VARIANTS) */
            /* If both TARGET and VARIANT extensions are disabled, there should be no colon */
            /* Doing nothing will leave kern = NULL, causing error condition below */
            VX_PRINT(VX_ZONE_ERROR, "Kernel name should not contain any ':' in this implementation\n");
#endif  /* defined(EXPERIMENTAL_USE_TARGET) || defined(EXPERIMENTAL_USE_VARIANTS) */
        }
        else if (colons == 2)
        {
#if defined(EXPERIMENTAL_USE_TARGET) && defined(EXPERIMENTAL_USE_VARIANTS)
            /* target:kernel:variant */
            vx_char *target = strtok(nameBuffer, ":");
            vx_char *kernel = strtok(NULL, ":");
            vx_char *variant = strtok(NULL,":");
            strncpy(targetName, target, VX_MAX_TARGET_NAME);
            strncpy(kernelName, kernel, VX_MAX_KERNEL_NAME);
            strncpy(variantName, variant, VX_MAX_VARIANT_NAME);
#else   /* defined(EXPERIMENTAL_USE_TARGET) && defined(EXPERIMENTAL_USE_VARIANTS) */
            /* If both TARGET and VARIANT extensions are disabled, there should be no colon */
            /* Doing nothing will leave kern = NULL, causing error condition below */
            VX_PRINT(VX_ZONE_ERROR, "Kernel name should not contain two ':' in this implementation\n");
#endif  /* defined(EXPERIMENTAL_USE_TARGET) && defined(EXPERIMENTAL_USE_VARIANTS) */
        }
        else
        {
            /* no extension supports > 2 colons so far */
            /* Doing nothing will leave kern = NULL, causing error condition below */
            VX_PRINT(VX_ZONE_ERROR, "Kernel name should not contain more than two ':' in this implementation\n");
        }

        free(nameBuffer);

#if defined(EXPERIMENTAL_USE_VARIANTS)
        VX_PRINT(VX_ZONE_KERNEL, "Scanning in set of %u kernels on %u targets.\n"
            "Target: %s\nKernel: %s\nVariant: %s\n",
            context->num_kernels, context->num_targets,
            targetName, kernelName, variantName);
#endif
        for (t = 0; t < context->num_targets && kern == NULL; t++)
        {
            vx_target_t *target = &context->targets[context->priority_targets[t]];
            if (target == NULL || target->enabled == vx_false_e)
                continue;
#if defined(EXPERIMENTAL_USE_VARIANTS)
            if (target->funcs.supports(target, targetName, kernelName, variantName, &k) == VX_SUCCESS)
#else
            if (target->funcs.supports(target, targetName, kernelName, &k) == VX_SUCCESS)
#endif
            {
                vx_kernel kernel = &target->kernels[k];
                vxPrintKernel(kernel);
                if (kernel->enabled == vx_true_e)
                {
                    kernel->affinity = context->priority_targets[t];
                    kern = kernel;
                    vxIncrementReference(&kern->base, VX_EXTERNAL);
                    break;
                }
            }
        }

        if (kern == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to find kernel %s\n", string);
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Failed to find kernel %s\n", string);
            kern = (vx_kernel_t *)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
        }
        else
        {
            VX_PRINT(VX_ZONE_KERNEL,"Found Kernel enum %d, name %s on target %s\n",
                kern->enumeration,
                kern->name,
                context->targets[kern->affinity].name);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid context %p\n", context);
    }
    return (vx_kernel)kern;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByEnum(vx_context context, vx_enum kernelenum)
{
    vx_kernel kernel = NULL;
    if (vxIsValidContext(context) == vx_true_e)
    {
        if (kernelenum < VX_KERNEL_INVALID)
        {
            vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "Invalid kernel enumeration (%d)\n", kernelenum);
            VX_PRINT(VX_ZONE_ERROR, "Invalid kernel enumeration (%x)\n", kernelenum);
        }
        else if (kernelenum >= VX_KERNEL_INVALID)
        {
            vx_uint32 k = 0u, t = 0u;
            VX_PRINT(VX_ZONE_KERNEL,"Scanning for kernel enum %d out of %d kernels\n", kernelenum, context->num_kernels);
            for (t = 0; t < context->num_targets; t++)
            {
                vx_target_t *target = &context->targets[context->priority_targets[t]];
                if (target == NULL || target->enabled == vx_false_e) {
                    VX_PRINT(VX_ZONE_KERNEL, "Target[%u] is not valid!\n", t);
                    continue;
                }
                VX_PRINT(VX_ZONE_KERNEL, "Checking Target[%u]=%s for %u kernels\n", context->priority_targets[t], target->name, target->num_kernels);
                for (k = 0; k < target->num_kernels; k++)
                {
                    if (target->kernels[k].enumeration == kernelenum)
                    {
                        kernel = &target->kernels[k];
                        kernel->affinity = context->priority_targets[t];
                        vxIncrementReference(&kernel->base, VX_EXTERNAL);
                        VX_PRINT(VX_ZONE_KERNEL,"Found Kernel[%u] enum:%d name:%s in target[%u]=%s\n", k, kernelenum, kernel->name, context->priority_targets[t], target->name);
                        break;
                    }
                }
            }
            if (kernel == NULL) {
                VX_PRINT(VX_ZONE_KERNEL, "Kernel enum %x not found.\n", kernelenum);
            }
        } else {
            VX_PRINT(VX_ZONE_ERROR, "Invalid enumeration %x\n", kernelenum);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid context %p\n", context);
    }
    return kernel;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseKernel(vx_kernel *kernel)
{
    vx_status status = VX_SUCCESS;
    vx_kernel kern = (kernel?*kernel:0);
    if (vxIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        VX_PRINT(VX_ZONE_KERNEL, "Releasing kernel "VX_FMT_REF"\n", (void *)kern);
        vxReleaseReferenceInt((vx_reference *)kernel, VX_TYPE_KERNEL, VX_EXTERNAL, NULL);
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Invalid Reference!\n");
    }
    return status;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernel(vx_context c,
                             vx_char name[VX_MAX_KERNEL_NAME],
                             vx_enum enumeration,
                             vx_kernel_f func_ptr,
                             vx_uint32 numParams,
                             vx_kernel_input_validate_f input,
                             vx_kernel_output_validate_f output,
                             vx_kernel_initialize_f initialize,
                             vx_kernel_deinitialize_f deinitialize)
{
    vx_context_t *context = (vx_context_t *)c;
    vx_kernel kernel = 0;
    vx_uint32 t = 0;
    vx_size index = 0;
    vx_target_t *target = NULL;
    vx_char targetName[VX_MAX_TARGET_NAME];

    if (vxIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Context\n");
        return (vx_kernel)NULL;
    }
    if (func_ptr == NULL ||
        input == NULL ||
        output == NULL ||
        numParams > VX_INT_MAX_PARAMS || numParams == 0 ||
        name == NULL ||
        strncmp(name, "",  VX_MAX_KERNEL_NAME) == 0)
        /* initialize and de-initialize can be NULL */
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Parameters!\n");
        vxAddLogEntry((vx_reference)c, VX_ERROR_INVALID_PARAMETERS, "Invalid Parameters supplied to vxAddKernel\n");
        return (vx_kernel)NULL;
    }

    /* find target to assign this to */
    index = strnindex(name, ':', VX_MAX_TARGET_NAME);
    if (index == VX_MAX_TARGET_NAME)
    {
        strcpy(targetName,"khronos.c_model");
    }
    else
    {
        strncpy(targetName, name, index);
    }
    VX_PRINT(VX_ZONE_KERNEL, "Deduced Name as %s\n", targetName);
    for (t = 0u; t < context->num_targets; t++)
    {
        target = &context->targets[t];
        if (strncmp(targetName,target->name, VX_MAX_TARGET_NAME) == 0)
        {
            break;
        }
        target = NULL;
    }
    if (target)
    {
        kernel = target->funcs.addkernel(target, name, enumeration,
                                         func_ptr, numParams,
                                         input, output,
                                         initialize, deinitialize);
        VX_PRINT(VX_ZONE_KERNEL,"Added Kernel %s to Target %s ("VX_FMT_REF")\n", name, target->name, kernel);
    }
    else
    {
        vxAddLogEntry((vx_reference)c, VX_ERROR_NO_RESOURCES, "No target named %s exists!\n", targetName);
    }
    return (vx_kernel)kernel;
}

#ifdef OPENVX_KHR_TILING
VX_API_ENTRY vx_kernel VX_API_CALL vxAddTilingKernel(vx_context c,
                            vx_char name[VX_MAX_KERNEL_NAME],
                            vx_enum enumeration,
                            vx_tiling_kernel_f flexible_func_ptr,
                            vx_tiling_kernel_f fast_func_ptr,
                            vx_uint32 num_params,
                            vx_kernel_input_validate_f input,
                            vx_kernel_output_validate_f output)
{
    vx_context_t *context = (vx_context_t *)c;
    vx_kernel kernel = 0;
    vx_uint32 t = 0;
    vx_size index = 0;
    vx_target_t *target = NULL;
    vx_char targetName[VX_MAX_TARGET_NAME];

    if (vxIsValidContext(context) == vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Context\n");
        return (vx_kernel)NULL;
    }
    if ((flexible_func_ptr == NULL && fast_func_ptr == NULL) ||
        input == NULL ||
        output == NULL ||
        num_params > VX_INT_MAX_PARAMS || num_params == 0 ||
        name == NULL ||
        strncmp(name, "",  VX_MAX_KERNEL_NAME) == 0)
        /* initialize and de-initialize can be NULL */
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Parameters!\n");
        vxAddLogEntry((vx_reference)c, VX_ERROR_INVALID_PARAMETERS, "Invalid Parameters supplied to vxAddKernel\n");
        return (vx_kernel)NULL;
    }

    /* find target to assign this to */
    index = strnindex(name, ':', VX_MAX_TARGET_NAME);
    if (index == VX_MAX_TARGET_NAME)
    {
        strcpy(targetName,"khronos.c_model");
    }
    else
    {
        strncpy(targetName, name, index);
    }
    VX_PRINT(VX_ZONE_KERNEL, "Deduced Name as %s\n", targetName);
    for (t = 0u; t < context->num_targets; t++)
    {
        target = &context->targets[t];
        if (strncmp(targetName,target->name, VX_MAX_TARGET_NAME) == 0)
        {
            break;
        }
        target = NULL;
    }
    if (target && target->funcs.addtilingkernel)
    {
        kernel = target->funcs.addtilingkernel(target, name, enumeration,
                                         flexible_func_ptr, fast_func_ptr, num_params,
                                         input, output);
        VX_PRINT(VX_ZONE_KERNEL,"Added Kernel %s to Target %s ("VX_FMT_REF")\n", name, target->name, kernel);
    }
    else
    {
        vxAddLogEntry((vx_reference)c, VX_ERROR_NO_RESOURCES, "No target named %s exists!\n", targetName);
    }
    return (vx_kernel)kernel;
}
#endif

VX_API_ENTRY vx_status VX_API_CALL vxFinalizeKernel(vx_kernel kernel)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        vx_uint32 p = 0;
        for (p = 0; p < VX_INT_MAX_PARAMS; p++)
        {
            if (p >= kernel->signature.num_parameters)
            {
                break;
            }
            if ((kernel->signature.directions[p] < VX_INPUT) ||
                (kernel->signature.directions[p] > VX_BIDIRECTIONAL))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
            }
            if (vxIsValidType(kernel->signature.types[p]) == vx_false_e)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                break;
            }
        }
        if (p == kernel->signature.num_parameters)
        {
            kernel->base.context->num_kernels++;
            if (vxIsKernelUnique(kernel) == vx_true_e) {
                VX_PRINT(VX_ZONE_KERNEL, "Kernel %s (%x) is unique!\n", kernel->name, kernel->enumeration);
                kernel->base.context->num_unique_kernels++;
            }
            kernel->enabled = vx_true_e;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryKernel(vx_kernel kern, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    vx_kernel_t *kernel = (vx_kernel_t *)kern;
    if (vxIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_KERNEL_ATTRIBUTE_PARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = kernel->signature.num_parameters;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_ATTRIBUTE_NAME:
                if (ptr != NULL && size <= VX_MAX_KERNEL_NAME)
                {
                    vx_char kname[VX_MAX_KERNEL_NAME];
                    vx_char *k, *v;
                    strncpy(kname, kernel->name, VX_MAX_KERNEL_NAME);
                    k = strtok(kname, ":");
                    v = strtok(NULL, ":");
                    (void)v; // need this variable in the future for variant searches
                    strncpy(ptr, k, VX_MAX_KERNEL_NAME);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_ATTRIBUTE_ENUM:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = kernel->enumeration;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_ATTRIBUTE_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = kernel->attributes.localDataSize;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#ifdef OPENVX_KHR_NODE_MEMORY
            case VX_KERNEL_ATTRIBUTE_GLOBAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = kernel->attributes.globalDataSize;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_KERNEL_ATTRIBUTE_GLOBAL_DATA_PTR:
                if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x1))
                {
                    *(vx_ptr_t *)ptr = kernel->attributes.globalDataPtr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#endif
#ifdef OPENVX_KHR_TILING
            case VX_KERNEL_ATTRIBUTE_INPUT_NEIGHBORHOOD:
                if (VX_CHECK_PARAM(ptr, size, vx_neighborhood_size_t, 0x3))
                {
                    memcpy(ptr, &kernel->attributes.nhbdinfo, size);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;

            case VX_KERNEL_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_tile_block_size_t, 0x3))
                {
                    memcpy(ptr, &kernel->attributes.blockinfo, size);
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
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAddParameterToKernel(vx_kernel kernel,
                                        vx_uint32 index,
                                        vx_enum dir,
                                        vx_enum data_type,
                                        vx_enum state)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_kernel_t *kern = (vx_kernel_t *)kernel;

    VX_PRINT(VX_ZONE_KERNEL,"INFO: Adding index %u, data_type 0x%x, dir:%d state:%d\n", index, data_type, dir, state);

    if (vxIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        if (index < kern->signature.num_parameters)
        {
#ifdef OPENVX_KHR_TILING
            if (kern->tiling_function)
            {
                if (((data_type != VX_TYPE_IMAGE) &&
                     (data_type != VX_TYPE_SCALAR)) ||
                    (vxIsValidDirection(dir) == vx_false_e) ||
                    (vxIsValidState(state) == vx_false_e))
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                else
                {
                    kern->signature.directions[index] = dir;
                    kern->signature.types[index] = data_type;
                    kern->signature.states[index] = state;
                    status = VX_SUCCESS;
                }
            }
            else
#endif
            {
                if ((vxIsValidType(data_type) == vx_false_e) ||
                    (vxIsValidDirection(dir) == vx_false_e) ||
                    (vxIsValidState(state) == vx_false_e))
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                else
                {
                    kern->signature.directions[index] = dir;
                    kern->signature.types[index] = data_type;
                    kern->signature.states[index] = state;
                    status = VX_SUCCESS;
                }
            }
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a valid reference!\n");
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveKernel(vx_kernel kernel)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_kernel_t *kern = (vx_kernel_t *)kernel;
    if (vxIsValidSpecificReference(&kern->base, VX_TYPE_KERNEL) == vx_true_e)
    {
        if (kern->enabled)
        {
            VX_PRINT(VX_ZONE_ERROR, "Can't remove finalized kernel\n");
            return VX_ERROR_INVALID_REFERENCE;
        }
        kern->enabled = vx_false_e;
        kern->enumeration = VX_KERNEL_INVALID;
        kern->base.context->num_kernels--;
        status = vxReleaseReferenceInt((vx_reference*)&kernel, VX_TYPE_KERNEL, VX_INTERNAL, NULL);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetKernelAttribute(vx_kernel k, vx_enum attribute, void * ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    vx_kernel_t *kernel = (vx_kernel_t *)k;

    if (vxIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (kernel->enabled == vx_true_e)
    {
        return VX_ERROR_NOT_SUPPORTED;
    }
    switch (attribute)
    {
        case VX_KERNEL_ATTRIBUTE_LOCAL_DATA_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                kernel->attributes.localDataSize = *(vx_size *)ptr;
                VX_PRINT(VX_ZONE_KERNEL, "Set Local Data Size to "VX_FMT_SIZE" bytes\n", kernel->attributes.localDataSize);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_KERNEL_ATTRIBUTE_LOCAL_DATA_PTR:
            if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x1))
            {
                kernel->attributes.localDataPtr = *(vx_ptr_t *)ptr;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
#ifdef EXPERIMENTAL_USE_NODE_MEMORY
        case VX_KERNEL_ATTRIBUTE_GLOBAL_DATA_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
            {
                kernel->attributes.globalDataSize = *(vx_size *)ptr;
                VX_PRINT(VX_ZONE_KERNEL, "Set Global Data Size to "VX_FMT_SIZE" bytes\n", kernel->attributes.globalDataSize);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_KERNEL_ATTRIBUTE_GLOBAL_DATA_PTR:
            if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x1))
            {
                kernel->attributes.globalDataPtr = *(vx_ptr_t *)ptr;
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
#endif
#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_ATTRIBUTE_INPUT_NEIGHBORHOOD:
            if (VX_CHECK_PARAM(ptr, size, vx_neighborhood_size_t, 0x3))
            {
                memcpy(&kernel->attributes.nhbdinfo, ptr, size);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_KERNEL_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE:
            if (VX_CHECK_PARAM(ptr, size, vx_tile_block_size_t, 0x3))
            {
                memcpy(&kernel->attributes.blockinfo, ptr, size);
            }
            else
            {
                status = VX_ERROR_INVALID_PARAMETERS;
            }
            break;
        case VX_KERNEL_ATTRIBUTE_BORDER:
            if (VX_CHECK_PARAM(ptr, size, vx_border_mode_t, 0x3))
            {
                vx_border_mode_t *border = (vx_border_mode_t *)ptr;
                if ((border->mode == VX_BORDER_MODE_SELF) ||
                    (border->mode == VX_BORDER_MODE_UNDEFINED))
                {
                    memcpy(&kernel->attributes.borders, border, sizeof(vx_border_mode_t));
                }
                else
                {
                    status = VX_ERROR_INVALID_VALUE;
                }
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
