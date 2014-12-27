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

/*!
 * \file vx_parameters.c
 * \example vx_parameters.c
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

void example_explore_parameters(vx_context context, vx_kernel kernel)
{
    vx_uint32 p, numParams = 0;
    vx_graph graph = vxCreateGraph(context);
    vx_node node = vxCreateGenericNode(graph, kernel);
    vxQueryKernel(kernel, VX_KERNEL_ATTRIBUTE_PARAMETERS, &numParams, sizeof(numParams));
    for (p = 0; p < numParams; p++)
    {
        //! [Getting the ref]
        vx_parameter param = vxGetParameterByIndex(node, p);
        vx_reference ref;
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &ref, sizeof(ref));
        //! [Getting the ref]
        if (ref)
        {
            //! [Getting the type]
            vx_enum type;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_TYPE, &type, sizeof(type));
            /* cast the ref to the correct vx_<type>. Atomics are now vx_scalar */
            //! [Getting the type]
        }
        vxReleaseParameter(&param);
    }
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
}
