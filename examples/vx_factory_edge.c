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

#include <stdio.h>
#include <VX/vx.h>
#include <VX/vx_helper.h>

/*! \file vx_factory_edge.c
 * \example vx_factory_edge.c
 * \brief An example graph factory.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */


/*! \brief An example Graph Factory which creates an edge graph.
 * \ingroup group_example
 */
vx_graph vxEdgeGraphFactory(vx_context c)
{
    vx_status status = VX_FAILURE;
    vx_graph g = 0;
    if (c)
    {
        vx_kernel kernels[] = {
            vxGetKernelByEnum(c, VX_KERNEL_GAUSSIAN_3x3),
            vxGetKernelByEnum(c, VX_KERNEL_SOBEL_3x3),
            vxGetKernelByEnum(c, VX_KERNEL_MAGNITUDE),
        };

        g = vxCreateGraph(c);
        if (g)
        {
            vx_image virts[] = {
                vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // blurred
                vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // gx
                vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // gy
            };
            vx_node nodes[] = {
                vxCreateGenericNode(g, kernels[0]), // Gaussian
                vxCreateGenericNode(g, kernels[1]), // Sobel
                vxCreateGenericNode(g, kernels[2]), // Mag
            };
            vx_parameter params[] = {
                vxGetParameterByIndex(nodes[0], 0), // input Gaussian
                vxGetParameterByIndex(nodes[2], 2), // output Mag
            };
            vx_uint32 p = 0;
            status  = VX_SUCCESS;
            for (p = 0; p < dimof(params); p++)
            {
                // the parameter inherits from the node its attributes
                status |= vxAddParameterToGraph(g, params[p]);
            }
            // configure linkage of the graph
            status |= vxSetParameterByIndex(nodes[0], 1, (vx_reference)virts[0]);
            status |= vxSetParameterByIndex(nodes[1], 0, (vx_reference)virts[0]);
            status |= vxSetParameterByIndex(nodes[1], 1, (vx_reference)virts[1]);
            status |= vxSetParameterByIndex(nodes[1], 2, (vx_reference)virts[2]);
            status |= vxSetParameterByIndex(nodes[2], 0, (vx_reference)virts[1]);
            status |= vxSetParameterByIndex(nodes[2], 1, (vx_reference)virts[2]);
            for (p = 0; p < dimof(virts); p++)
            {
                /* this just decreases the external reference count */
                vxReleaseImage(&virts[p]);
            }
            if (status != VX_SUCCESS)
            {
                printf("Failed to create graph in factory!\n");
                vxReleaseGraph(&g);
                g = 0;
            }
        }
    }
    return g;
}

