/*
 * Copyright (c) 2012-2014 The Khronos Group Inc.
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

#ifndef _VX_SAMPLE_H_
#define _VX_SAMPLE_H_

/*!
 * \file
 * \brief The Sample Extensions (which may end up in later revisions)
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 */

enum _vx_sample_kernels_e {
    VX_KERNEL_SAMPLE_BASE = VX_KERNEL_BASE(VX_ID_KHRONOS, 0),

    /*! \brief */
    VX_KERNEL_SAMPLE_CHILD_GRAPH,
};
#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a child graph within a node of a parent graph.
 * \param [in] parent The parent graph.
 * \param [in] child The child graph.
 * \note Child graphs do not have a strictly defined data interface like
 * kernels and as such, lack a "signature". However, data does go in and out
 * like a kernel. When a graph is imported as a node into a parent, a parameter
 * list is generated which contains the data objects which are singularly referenced
 * as VX_INPUT or VX_OUTPUT or VX_BIDIRECTION
 * (there can be only 1 referent in the total graph here too). The parameter list
 * order is first all inputs, then bi-directional, then outputs, by order of which nodes
 * where declared in the child graph.
 * \ingroup group_int_graph
 */
vx_node vxCreateNodeFromGraph(vx_graph parent, vx_graph child);

#ifdef __cplusplus
}
#endif

#endif /* _VX_SAMPLE_H_ */

