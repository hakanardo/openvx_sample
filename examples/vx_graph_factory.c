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
 * \file
 * \brief The example main code.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vx_graph_factory.h>

/*! \brief The array of factory functions.
 * \ingroup group_example
 */
vx_graph_factory_t factories[] = {
    {VX_GRAPH_FACTORY_EDGE, vxEdgeGraphFactory},
    {VX_GRAPH_FACTORY_CORNERS, vxCornersGraphFactory},
    {VX_GRAPH_FACTORY_PIPELINE, vxPipelineGraphFactory},
};

/*! \brief Returns a graph from one of the matching factories.
 * \ingroup group_example
 */
vx_graph vxGraphFactory(vx_context context, vx_enum factory_name)
{
    vx_uint32 f = 0;
    for (f = 0; f < dimof(factories); f++)
    {
        if (factory_name == factories[f].factory_name)
        {
            return factories[f].factory(context);
        }
    }
    return 0;
}

/*! \brief The graph factory example.
 * \ingroup group_example
 */
int main(int argc, char *argv[])
{
    vx_status status = VX_SUCCESS;
    vx_context context = vxCreateContext();
    if (context)
    {
        vx_image images[] = {
                vxCreateImage(context, 640, 480, VX_DF_IMAGE_U8),
                vxCreateImage(context, 640, 480, VX_DF_IMAGE_S16),
        };
        vx_graph graph = vxGraphFactory(context, VX_GRAPH_FACTORY_EDGE);
        if (graph)
        {
            vx_uint32 p, num = 0;
            status |= vxQueryGraph(graph, VX_GRAPH_ATTRIBUTE_NUMPARAMETERS, &num, sizeof(num));
            if (status == VX_SUCCESS)
            {
                printf("There are %u parameters to this graph!\n", num);
                for (p = 0; p < num; p++)
                {
                    vx_parameter param = vxGetGraphParameterByIndex(graph, p);
                    if (param)
                    {
                        vx_enum dir = 0;
                        vx_enum type = 0;
                        status |= vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_DIRECTION, &dir, sizeof(dir));
                        status |= vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_TYPE, &type, sizeof(type));
                        printf("graph.parameter[%u] dir:%d type:%08x\n", p, dir, type);
                        vxReleaseParameter(&param);
                    }
                    else
                    {
                        printf("Invalid parameter retrieved from graph!\n");
                    }
                }

                status |= vxSetGraphParameterByIndex(graph, 0, (vx_reference)images[0]);
                status |= vxSetGraphParameterByIndex(graph, 1, (vx_reference)images[1]);
            }

            status |= vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
                if (status == VX_SUCCESS)
                {
                    printf("Ran Graph!\n");
                }
            }
            vxReleaseGraph(&graph);
        }
        else
        {
            printf("Failed to create graph!\n");
        }
        vxReleaseContext(&context);
    }
    else
    {
        printf("failed to create context!\n");
    }
    return status;
}

