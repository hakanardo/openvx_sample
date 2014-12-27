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

vx_status run_example_parallel(void)
{
    vx_status status = VX_SUCCESS;
    //! [independent]
    vx_context context = vxCreateContext();
    vx_image images[] = {
            vxCreateImage(context, 640, 480, VX_DF_IMAGE_UYVY),
            vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8),
            vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8),
    };
    vx_graph graph = vxCreateGraph(context);
    vx_image virts[] = {
            vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
            vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
            vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
            vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),
    };

    vxChannelExtractNode(graph, images[0], VX_CHANNEL_Y, virts[0]),
    vxGaussian3x3Node(graph, virts[0], virts[1]),
    vxSobel3x3Node(graph, virts[1], virts[2], virts[3]),
    vxMagnitudeNode(graph, virts[2], virts[3], images[1]),
    vxPhaseNode(graph, virts[2], virts[3], images[2]),

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    vxReleaseContext(&context); /* this will release everything */
    //! [independent]
    return status;
 }
