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

#ifndef _VX_EXAMPLE_GRAPH_FACTORY_H_
#define _VX_EXAMPLE_GRAPH_FACTORY_H_

#include <VX/vx.h>
#include <VX/vx_helper.h>

/*! \ingroup group_example */
enum vx_factory_name_e {
    VX_GRAPH_FACTORY_EDGE,
    VX_GRAPH_FACTORY_CORNERS,
    VX_GRAPH_FACTORY_PIPELINE,
};

/*! \brief An prototype of a graph factory method.
 * \ingroup group_example
 */
typedef vx_graph(*vx_graph_factory_f)(vx_context context);

/*! \brief A graph factory structure.
 * \ingroup group_example
 */
typedef struct _vx_graph_factory_t {
    vx_enum factory_name;
    vx_graph_factory_f factory;
} vx_graph_factory_t;

// PROTOTYPES
vx_graph vxEdgeGraphFactory(vx_context context);
vx_graph vxCornersGraphFactory(vx_context context);
vx_graph vxPipelineGraphFactory(vx_context context);

#endif
