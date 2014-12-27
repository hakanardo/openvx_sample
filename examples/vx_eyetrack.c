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

/*!
 * \file vx_eyetrack.c
 * \example vx_eyetrack.c
 * \brief The example graph used to preprocess eye tracking data.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <vx_examples.h>

/*!
 * \brief An example of an Eye Tracking Graph.
 * \ingroup group_example
 */
int example_eyetracking(int argc, char *argv[])
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 640;
    vx_uint32 height = 480;
    vx_bool running = vx_true_e;
    vx_context context = vxCreateContext();
    if (context)
    {
        vx_image images[] = {
            vxCreateImage(context, width, height, VX_DF_IMAGE_UYVY),
            vxCreateImage(context, width, height, VX_DF_IMAGE_U8),
        };

        vx_graph graph = vxCreateGraph(context);
        if (graph)
        {
            vx_node nodes[] = {
                /*! \todo add nodes which process eye tracking */
            };

            status = vxVerifyGraph(context, graph);
            if (status == VX_SUCCESS)
            {
                do {
                    /*! \todo capture an image */
                    status = vxProcessGraph(context, graph, NULL);
                    /*! \todo do something with the data */
                } while (running == vx_true_e);
            }
            vxReleaseGraph(&graph);
        }
        vxReleaseContext(&context);
    }
    return status;
}

