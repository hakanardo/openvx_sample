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
#include <VX/vxu.h>

vx_status vxSingleNodeGraph()
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 320, height = 240;
    //! [whole]
    /* This is an example of a Single Node Graph */
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
    //! [node]
    vx_node node = vxGaussian3x3Node(graph, in, out);
    //! [node]

    //! [verify]
    status = vxVerifyGraph(graph);
    //! [verify]
    //! [process]
    if (status == VX_SUCCESS)
        status = vxProcessGraph(graph);
    //! [process]
    //! [whole]
    //! [vxu]
    status = vxuGaussian3x3(context, in, out);
    //! [vxu]

    //! [teardown]
    vxReleaseNode(&node);
    /* this also releases any nodes we forgot to release */
    vxReleaseGraph(&graph);
    vxReleaseImage(&in);
    vxReleaseImage(&out);
    /* this catches anything else we forgot to release */
    vxReleaseContext(&context);
    //! [teardown]
    return status;
}
