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

//! [callback]
#define MY_DESIRED_THRESHOLD (10)
vx_action VX_CALLBACK analyze_brightness(vx_node node) {
    // extract the max value
    vx_action action = VX_ACTION_ABANDON;
    vx_parameter pmax = vxGetParameterByIndex(node, 2); // Max Value
    if (pmax) {
        vx_scalar smax = 0;
        vxQueryParameter(pmax, VX_PARAMETER_ATTRIBUTE_REF, &smax, sizeof(smax));
        if (smax) {
            vx_uint8 value = 0u;
            vxAccessScalarValue(smax, &value);
            if (value >= MY_DESIRED_THRESHOLD) {
                action = VX_ACTION_CONTINUE;
            }
            vxReleaseScalar(&smax);
        }
        vxReleaseParameter(&pmax);
    }
    return action;
}
//! [callback]

vx_status vx_example_callback(vx_context context, vx_image input, vx_image output) {
    vx_status status = VX_SUCCESS;
    //! [graph setup]
    vx_graph graph = vxCreateGraph(context);
    if (graph) {
        vx_uint8 lmin = 0, lmax = 0;
        vx_uint32 minCount = 0, maxCount = 0;
        vx_scalar scalars[] = {
            vxCreateScalar(context, VX_TYPE_UINT8, &lmin),
            vxCreateScalar(context, VX_TYPE_UINT8, &lmax),
            vxCreateScalar(context, VX_TYPE_UINT32, &minCount),
            vxCreateScalar(context, VX_TYPE_UINT32, &maxCount),
        };
        vx_array arrays[] = {
            vxCreateArray(context, VX_TYPE_COORDINATES2D, 1),
            vxCreateArray(context, VX_TYPE_COORDINATES2D, 1)
        };
        vx_node nodes[] = {
            vxMinMaxLocNode(graph, input, scalars[0], scalars[1], arrays[0], arrays[1], scalars[2], scalars[3]),
            /// other nodes
        };
        status = vxAssignNodeCallback(nodes[0], &analyze_brightness);
        // do other
    }
    //! [graph setup]
    return status;
}
