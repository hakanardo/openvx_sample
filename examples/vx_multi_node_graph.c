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
#include <VX/vx_helper.h>

vx_status vx_example_multinode_graph()
{
    vx_status status = VX_SUCCESS;
    vx_uint32 n, width = 320, height = 240;
    //! [context]
    vx_context context = vxCreateContext();
    //! [context]
    //! [data]
    vx_image in = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vx_image out = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    //! [data]
    //! [graph]
    vx_graph graph = vxCreateGraph(context);
    //! [graph]
    //! [virt]
    vx_image blurred = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT);
    vx_image gx = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16);
    vx_image gy = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16);
    //! [virt]
    //! [nodes]
    vx_node nodes[] = {
            vxGaussian3x3Node(graph, in, blurred),
            vxSobel3x3Node(graph, blurred, gx, gy),
            vxMagnitudeNode(graph, gx, gy, out),
    };
    //! [nodes]

    //! [verify]
    status = vxVerifyGraph(graph);
    //! [verify]

    //! [process]
    if (status == VX_SUCCESS)
        status = vxProcessGraph(graph);
    //! [process]

    //! [teardown]
    for (n = 0; n < dimof(nodes); n++)
        vxReleaseNode(&nodes[n]);
    /* this catches anything else we forgot to release */
    vxReleaseContext(&context);
    //! [teardown]
    return status;
}
