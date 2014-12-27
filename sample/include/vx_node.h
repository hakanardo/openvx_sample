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

#ifndef _OPENVX_INT_NODE_H_
#define _OPENVX_INT_NODE_H_

/*!
 * \file
 * \brief The internal node implementation.
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_node Internal Node API
 * \ingroup group_internal
 * \brief The internal Node API.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief The internal node attributes
 * \ingroup group_int_node
 */
enum vx_node_attribute_internal_e {
    /*\brief The attribute used to get the pointer to tile local memory */
    VX_NODE_ATTRIBUTE_TILE_MEMORY_PTR = VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_NODE) + 0xD,
};

/*! \brief Node parameter setter, no check.
 * \ingroup group_int_node
 */
void vxNodeSetParameter(vx_node node, vx_uint32 index, vx_reference value);

 /*! \brief Used to print the values of the node.
  * \ingroup group_int_node
  */
void vxPrintNode(vx_node node);

/*! \brief Used to completely destroy a node.
 * \ingroup group_int_node
 */
void vxDestructNode(vx_reference ref);

/*! \brief Used to remove a node from a graph.
 * \ingroup group_int_node
 */
void vxRemoveNodeInt(vx_node *n);

/*! \brief Used to set the graph as a child of the node within another graph.
 * \param [in] n The node.
 * \param [in] g The child graph.
 * \retval VX_ERROR_INVALID_GRAPH The Graph's parameters do not match the Node's
 * parameters.
 * \ingroup group_int_node
 */
vx_status vxSetChildGraphOfNode(vx_node node, vx_graph graph);

/*! \brief Retrieves the handle of the child graph, if it exists.
 * \param [in] n The node.
 * \return Returns the handle of the child graph or zero if it doesn't have one.
 * \ingroup group_int_node
 */
vx_graph vxGetChildGraphOfNode(vx_node node);

#ifdef __cplusplus
}
#endif

#endif
