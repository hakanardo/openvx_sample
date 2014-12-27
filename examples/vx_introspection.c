/*
 * Copyright (c) 2013-2014 The Khronos Group Inc.
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

#include <VX/vx.h>
#include <stdlib.h>

#if defined(EXPERIMENTAL_USE_TARGET)
#include <VX/vx_ext_target.h>

vx_status vx_example_introspection()
{
    //! [context]
    vx_context context = vxCreateContext();
    //! [context]

    //! [targets]
    vx_uint32 num_targets = 0;
    vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_TARGETS, &num_targets, sizeof(num_targets));
    //! [targets]
    vx_uint32 t, k, p;
    //! [target_loop]
    for (t = 0; t < num_targets; t++)
    //! [target_loop]
    {
        //! [target_by_index]
        vx_target target = vxGetTargetByIndex(context, t);
        //! [target_by_index]

        //! [target_name]
        vx_char target_name[VX_MAX_TARGET_NAME];
        status = vxQueryTarget(target, VX_TARGET_ATTRIBUTE_NAME, target_name, sizeof(target_name));
        //! [target_name]
        if (status == VX_SUCCESS) {
        //! [num]
        vx_size num_kernels = 0ul;
        status = vxQueryTarget(target, VX_TARGET_ATTRIBUTE_NUMKERNELS, &num_kernels, sizeof(num_kernels));
        //! [num]
        if (num_kernels > 0ul) {
        //! [malloc]
        vx_kernel_info_t *table = (vx_kernel_info_t *)calloc(num_kernels, sizeof(vx_kernel_info_t));
        status = vxQueryTarget(target, VX_TARGET_ATTRIBUTE_KERNELTABLE, table, num_kernels*sizeof(vx_kernel_info_t));
        //! [malloc]
        //! [kernel_loop]
        for (k = 0; k < num_kernels; k++)
        //! [kernel_loop]
        {
            //! [kernel]
            vx_kernel kernel = vxGetKernelByName(context, table[k].name);
            //! [kernel]

            //! [kernel_attr]
            vx_uint32 num_params = 0u;
            vx_char kernel_name[VX_MAX_KERNEL_NAME];
            status = vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_PARAMETERS, &num_params, sizeof(num_params));
            status = vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_NAME, kernel_name, sizeof(kernel_name));
            //! [kernel_attr]
            {
                //! [parameter_loop]
                vx_graph graph = vxCreateGraph(context);
                vx_node node = vxCreateGenericNode(graph, kernel);
                for (p = 0; p < num_params; p++)
                //! [parameter_loop]
                {
                    //! [parameter]
                    vx_parameter param = vxGetParameterByIndex(node, p);
                    //! [parameter]

                    //! [parameter_attr]
                    vx_enum dir, state, type;
                    status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_DIRECTION, &dir, sizeof(dir));
                    status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_STATE, &state, sizeof(state));
                    status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_TYPE, &type, sizeof(type));
                    //! [parameter_attr]
                }
                vxReleaseNode(&node);
                vxReleaseGraph(&graph);
            }
            vxReleaseKernel(&kernel);
        }
        }
        }
        vxReleaseTarget(&target);
    }
    //! [teardown]
    vxReleaseContext(&context);
    //! [teardown]
    return status;
}
#else

vx_status vx_example_introspection()
{
    //! [context]
    vx_context context = vxCreateContext();
    //! [context]
    //! [num]
    vx_size k, num_kernels = 0ul;
    vx_status status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNELS, &num_kernels, sizeof(num_kernels));
    //! [num]
    if (num_kernels > 0ul) {
    //! [malloc]
    vx_size size = num_kernels * sizeof(vx_kernel_info_t);
    vx_kernel_info_t *table = (vx_kernel_info_t *)malloc(size);
    status = vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNEL_TABLE, table, size);
    //! [malloc]

    //! [kernel_loop]
    for (k = 0; k < num_kernels; k++)
    //! [kernel_loop]
    {
        //! [kernel]
        vx_kernel kernel = vxGetKernelByName(context, table[k].name);
        //! [kernel]
        //! [kernel_attr]
        vx_uint32 p, num_params = 0u;
        status = vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_PARAMETERS, &num_params, sizeof(num_params));
        //! [kernel_attr]
        if (status == VX_SUCCESS)
        {
            //! [parameter_loop]
            for (p = 0; p < num_params; p++)
            //! [parameter_loop]
            {
                //! [parameter]
                vx_parameter param = vxGetKernelParameterByIndex(kernel, p);
                //! [parameter]

                //! [parameter_attr]
                vx_enum dir, state, type;
                status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_DIRECTION, &dir, sizeof(dir));
                status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_STATE, &state, sizeof(state));
                status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_TYPE, &type, sizeof(type));
                //! [parameter_attr]
            }
        }
        vxReleaseKernel(&kernel);
    }
    }
    //! [teardown]
    vxReleaseContext(&context);
    //! [teardown]
    return status;
}

#endif
