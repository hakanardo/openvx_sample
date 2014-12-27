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

void example_delaygraph(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_image yuv = vxCreateImage(context, 320, 240, VX_DF_IMAGE_UYVY);
    vx_delay delay = vxCreateDelay(context, (vx_reference)yuv, 4);
    vx_image rgb = vxCreateImage(context, 320, 240, VX_DF_IMAGE_RGB);
    vx_graph graph = vxCreateGraph(context);

    vxColorConvertNode(graph, (vx_image)vxGetReferenceFromDelay(delay, 0),rgb);

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        do {
             /* capture or read image into vxGetImageFromDelay(delay, 0); */
             status = vxProcessGraph(graph);
             /* 0 becomes -1, -1 becomes -2, etc. convert is updated with new 0 */
             vxAgeDelay(delay);
         } while (1);
    }
}
