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

/*!
 * \file
 * \brief The OpenMP Target Interface
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <vx_interface.h>

static const vx_char name[VX_MAX_TARGET_NAME] = "khronos.openmp";

/*! \brief List of Kernel supported by OpenMP Target.
 */
static vx_kernel_description_t *target_kernels[] = {
        &lut_kernel,
};

/*! \brief Declares the number of base supported kernels.
 * \ingroup group_implementation
 */
static vx_uint32 num_target_kernels = dimof(target_kernels);

/******************************************************************************/
/* EXPORTED FUNCTIONS */
/******************************************************************************/

vx_status vxTargetInit(vx_target target)
{
    if (target)
    {
        strncpy(target->name, name, VX_MAX_TARGET_NAME);
        target->priority = VX_TARGET_PRIORITY_OPENMP;
    }
    return vxInitializeTarget(target, target_kernels, num_target_kernels);
}

vx_status vxTargetDeinit(vx_target target)
{
    return vxDeinitializeTarget(target);
}

vx_status vxTargetSupports(vx_target target,
                           vx_char targetName[VX_MAX_TARGET_NAME],
                           vx_char kernelName[VX_MAX_KERNEL_NAME],
#if defined(EXPERIMENTAL_USE_VARIANTS)
                           vx_char variantName[VX_MAX_VARIANT_NAME],
#endif
                           vx_uint32 *pIndex)
{
    vx_status status = VX_ERROR_NOT_SUPPORTED;
    if (strncmp(targetName, name, VX_MAX_TARGET_NAME) == 0 ||
        strncmp(targetName, "default", VX_MAX_TARGET_NAME) == 0)
    {
        vx_uint32 k = 0u;
        for (k = 0u; k < target->num_kernels; k++)
        {
            vx_char targetKernelName[VX_MAX_KERNEL_NAME];
            vx_char *kernel;
#if defined(EXPERIMENTAL_USE_VARIANTS)
            vx_char *variant;
            vx_char def[8] = "default";
#endif

            strncpy(targetKernelName, target->kernels[k].name, VX_MAX_KERNEL_NAME);
            kernel = strtok(targetKernelName, ":");
#if defined(EXPERIMENTAL_USE_VARIANTS)
            variant = strtok(NULL, ":");

            if (variant == NULL)
                variant = def;
#endif
            if (strncmp(kernelName, kernel, VX_MAX_KERNEL_NAME) == 0
#if defined(EXPERIMENTAL_USE_VARIANTS)
                && strncmp(variantName, variant, VX_MAX_VARIANT_NAME) == 0
#endif
                )
            {
                status = VX_SUCCESS;
                if (pIndex) *pIndex = k;
                break;
            }
        }
    }
    return status;
}

vx_action vxTargetProcess(vx_target target, vx_node_t *nodes[], vx_size startIndex, vx_size numNodes)
{
    vx_action action = VX_ACTION_CONTINUE;
    vx_status status = VX_SUCCESS;
    vx_size n = 0;
    for (n = startIndex; (n < (startIndex + numNodes)) && (action == VX_ACTION_CONTINUE); n++)
    {
        VX_PRINT(VX_ZONE_GRAPH,"Executing Kernel %s:%d in Nodes[%u] on target %s\n",
            nodes[n]->kernel->name,
            nodes[n]->kernel->enumeration,
            n,
            nodes[n]->base.context->targets[nodes[n]->affinity].name);

        vxStartCapture(&nodes[n]->perf);
        status = nodes[n]->kernel->function((vx_node)nodes[n],
                                            (vx_reference *)nodes[n]->parameters,
                                            nodes[n]->kernel->signature.num_parameters);
        nodes[n]->executed = vx_true_e;
        nodes[n]->status = status;
        vxStopCapture(&nodes[n]->perf);

        VX_PRINT(VX_ZONE_GRAPH,"kernel %s returned %d\n", nodes[n]->kernel->name, status);

        if (status == VX_SUCCESS)
        {
            /* call the callback if it is attached */
            if (nodes[n]->callback)
            {
                action = nodes[n]->callback((vx_node)nodes[n]);
                VX_PRINT(VX_ZONE_GRAPH,"callback returned action %d\n", action);
            }
        }
        else
        {
            action = VX_ACTION_ABANDON;
            VX_PRINT(VX_ZONE_ERROR, "Abandoning Graph due to error (%d)!\n", status);
        }
    }
    return action;
}

vx_status vxTargetVerify(vx_target target, vx_node_t *node)
{
    vx_status status = VX_SUCCESS;
    return status;
}

vx_kernel vxTargetAddKernel(vx_target target,
                            vx_char name[VX_MAX_KERNEL_NAME],
                            vx_enum enumeration,
                            vx_kernel_f func_ptr,
                            vx_uint32 numParams,
                            vx_kernel_input_validate_f input,
                            vx_kernel_output_validate_f output,
                            vx_kernel_initialize_f initialize,
                            vx_kernel_deinitialize_f deinitialize)
{
    vx_uint32 k = 0u;
    vx_kernel_t *kernel = NULL;
    for (k = target->num_kernels; k < VX_INT_MAX_KERNELS; k++)
    {
        kernel = &(target->kernels[k]);
        if ((kernel->enabled == vx_false_e) &&
            (kernel->enumeration == VX_KERNEL_INVALID))
        {
            vxInitializeKernel(target->base.context,
                               kernel,
                               enumeration, func_ptr, name,
                               NULL, numParams,
                               input, output, initialize, deinitialize);
            VX_PRINT(VX_ZONE_KERNEL, "Reserving %s Kernel[%u] for %s\n", target->name, k, kernel->name);
            target->num_kernels++;
            break;
        }
        kernel = NULL;
    }
    return (vx_kernel)kernel;
}

