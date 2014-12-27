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

#include <vx_internal.h>

void vxPrintNode(vx_node_t *node)
{
    if (node)
        VX_PRINT(VX_ZONE_NODE, "vx_node_t:%p %s:%d affinity:%s\n",
            node,
            node->kernel->name,
            node->kernel->enumeration,
            node->base.context->targets[node->affinity].name);
}

void vxNodeSetParameter(vx_node node, vx_uint32 index, vx_reference value)
{
    if (node->parameters[index]) {
        vxReleaseReferenceInt(&node->parameters[index], node->parameters[index]->type, VX_INTERNAL, NULL);
    }

    vxIncrementReference(value, VX_INTERNAL);
    node->parameters[index] = (vx_reference_t *)value;
}

VX_API_ENTRY vx_node VX_API_CALL vxCreateGenericNode(vx_graph g, vx_kernel k)
{
    vx_node_t *node = NULL;
    vx_graph_t *graph = (vx_graph_t *)g;
    vx_kernel_t *kernel = (vx_kernel_t *)k;
    if ((vxIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e) &&
        (vxIsValidSpecificReference(&kernel->base, VX_TYPE_KERNEL) == vx_true_e))
    {
        vx_uint32 n = 0;
        vxSemWait(&graph->base.lock);
        for (n = 0; n < VX_INT_MAX_REF; n++)
        {
            if (graph->nodes[n] == NULL)
            {
                node = (vx_node)vxCreateReference(graph->base.context, VX_TYPE_NODE, VX_EXTERNAL, &graph->base);
                if (node && node->base.type == VX_TYPE_NODE)
                {
                    /* reference the abstract kernel. */
                    node->kernel = kernel;
                    node->affinity = kernel->affinity;

                    /* show that there are potentially multiple nodes using this kernel. */
                    vxIncrementReference(&kernel->base, VX_INTERNAL);

                    /* copy the attributes over */
                    memcpy(&node->attributes, &kernel->attributes, sizeof(vx_kernel_attr_t));

                    /* setup our forward and back references to the node/graph */
                    graph->nodes[n] = node;
                    node->graph = graph;
                    vxIncrementReference(&node->base, VX_INTERNAL); /* one for the graph */

                    /* increase the count of nodes in the graph. */
                    graph->numNodes++;

                    vxInitPerf(&graph->nodes[n]->perf);

                    /* force a re-verify */
                    graph->verified = vx_false_e;

                    VX_PRINT(VX_ZONE_NODE, "Created Node %p %s affinity:%s\n", node, node->kernel->name, node->base.context->targets[node->affinity].name);
                }
                break; /* succeed or fail, break. */
            }
        }
        vxSemPost(&graph->base.lock);
        vxPrintReference((vx_reference )node);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Either graph %p or kernel %p was invalid!\n", graph, kernel);
        vxAddLogEntry((vx_reference)g, VX_ERROR_INVALID_REFERENCE, "Either graph %p or kernel %p was invalid!\n", graph, kernel);
        node = (vx_node_t *)vxGetErrorObject(graph->base.context, VX_ERROR_INVALID_REFERENCE);
    }
    return (vx_node)node;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryNode(vx_node n, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    vx_node_t *node = (vx_node_t *)n;
    if (vxIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_NODE_ATTRIBUTE_PERFORMANCE:
                if (VX_CHECK_PARAM(ptr, size, vx_perf_t, 0x3))
                {
                    memcpy(ptr, &node->perf, size);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_STATUS:
                if (VX_CHECK_PARAM(ptr, size, vx_status, 0x3))
                {
                    *(vx_status *)ptr = node->status;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = node->attributes.localDataSize;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR:
                if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x3))
                {
                    *(vx_ptr_t *)ptr = node->attributes.localDataPtr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#ifdef OPENVX_KHR_NODE_MEMORY
            case VX_NODE_ATTRIBUTE_GLOBAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = node->attributes.globalDataSize;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_GLOBAL_DATA_PTR:
                if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x3))
                {
                    *(vx_ptr_t *)ptr = node->attributes.globalDataPtr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#endif
            case VX_NODE_ATTRIBUTE_BORDER_MODE:
                if (VX_CHECK_PARAM(ptr, size, vx_border_mode_t, 0x3))
                {
                    VX_PRINT(VX_ZONE_NODE, "Border mode %x set!\n", node->attributes.borders.mode);
                    memcpy((vx_border_mode_t *)ptr, &node->attributes.borders, sizeof(vx_border_mode_t));
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#ifdef OPENVX_KHR_TILING
            case VX_NODE_ATTRIBUTE_INPUT_NEIGHBORHOOD:
                if (VX_CHECK_PARAM(ptr, size, vx_neighborhood_size_t, 0x3))
                {
                    memcpy(ptr, &node->kernel->attributes.nhbdinfo, size);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_tile_block_size_t, 0x3))
                {
                    memcpy(ptr, &node->kernel->attributes.blockinfo, size);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_TILE_MEMORY_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = node->attributes.tileDataSize;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_TILE_MEMORY_PTR:
                if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x3))
                {
                    *(vx_ptr_t *)ptr = node->attributes.tileDataPtr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
#endif
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    VX_PRINT(VX_ZONE_API, "Returned %d\n", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeAttribute(vx_node n, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    vx_node_t *node = (vx_node_t *)n;
    if (vxIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e)
    {
        if (node->graph->verified == vx_true_e)
        {
            return VX_ERROR_NOT_SUPPORTED;
        }
        switch (attribute)
        {
            case VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    node->attributes.localDataSize = *(vx_size *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR:
                if (VX_CHECK_PARAM(ptr, size, vx_ptr_t, 0x3))
                {
                    node->attributes.localDataPtr = *(vx_ptr_t *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_NODE_ATTRIBUTE_BORDER_MODE:
                if (VX_CHECK_PARAM(ptr, size, vx_border_mode_t, 0x3))
                {
#ifdef OPENVX_KHR_TILING
                    if (node->attributes.borders.mode == VX_BORDER_MODE_SELF)
                    {
                        status = VX_ERROR_INVALID_VALUE;
                    }
                    else
#endif
                    {
                        memcpy(&node->attributes.borders, (vx_border_mode_t *)ptr, sizeof(vx_border_mode_t));
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

void vxDestructNode(vx_reference ref)
{
    vx_uint32 p = 0;
    vx_node node = (vx_node)ref;

    if (node->kernel == NULL)
    {
        VX_PRINT(VX_ZONE_WARNING, "Node has no kernel!\n");
        DEBUG_BREAK();
    }

    /* de-initialize the kernel */
    if (node->kernel->deinitialize)
    {
        vx_status status;
        status = node->kernel->deinitialize((vx_node)node,
                                            (vx_reference *)node->parameters,
                                            node->kernel->signature.num_parameters);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to de-initialize kernel %s!\n", node->kernel->name);
        }
    }

    /* remove, don't delete, all references from the node itself */
    for (p = 0; p < node->kernel->signature.num_parameters; p++)
    {
        vx_reference ref = node->parameters[p];
        if (ref)
        {
            /* Remove the potential delay association */
            if (ref->delay!=NULL) {
                vx_bool res = vxRemoveAssociationToDelay(ref, node, p);
                if (res == vx_false_e) {
                    VX_PRINT(VX_ZONE_ERROR, "Internal error removing delay association\n");
                }
            }
            vxReleaseReferenceInt(&ref, ref->type, VX_INTERNAL, NULL);
            node->parameters[p] = NULL;
        }
    }

    /* free the local memory */
    if (node->attributes.localDataPtr)
    {
        free(node->attributes.localDataPtr);
        node->attributes.localDataPtr = NULL;
    }

    vxReleaseReferenceInt((vx_reference *)&node->kernel, VX_TYPE_KERNEL, VX_INTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseNode(vx_node *n)
{
    return vxReleaseReferenceInt((vx_reference *)n, VX_TYPE_NODE, VX_EXTERNAL, NULL);
}

void vxRemoveNodeInt(vx_node *n)
{
    vx_node_t *node = (vx_node_t *)(n?*n:0);
    if (node && vxIsValidSpecificReference(&node->base, VX_TYPE_NODE))
    {
        if (node->graph)
        {
            vx_uint32 i = 0;
            vx_bool removedFromGraph = vx_false_e;
            vxSemWait(&node->graph->base.lock);
            /* remove the reference from the graph */
            for (i = 0; i < node->graph->numNodes; i++)
            {
                if (node->graph->nodes[i] == node)
                {
                    node->graph->numNodes--;
                    node->graph->nodes[i] = node->graph->nodes[node->graph->numNodes];
                    node->graph->nodes[node->graph->numNodes] = NULL;
                    /* force the graph to be verified again */
                    node->graph->verified = vx_false_e;
                    removedFromGraph = vx_true_e;
                    break;
                }
            }
            vxSemPost(&node->graph->base.lock);
            /* If this node is within a graph, release internal reference to graph */
            if(removedFromGraph) {
                vxReleaseReferenceInt((vx_reference *)&node, VX_TYPE_NODE, VX_INTERNAL, NULL);
            }
        }
    }
}

VX_API_ENTRY void VX_API_CALL vxRemoveNode(vx_node *n)
{
    vx_node_t *node = (vx_node_t *)(n?*n:0);
    if (node && vxIsValidSpecificReference(&node->base, VX_TYPE_NODE))
    {
        vxRemoveNodeInt(n);
        vxReleaseReferenceInt((vx_reference *)&node, VX_TYPE_NODE, VX_EXTERNAL, NULL);
        *n = NULL;
    }
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeCallback(vx_node n, vx_nodecomplete_f callback)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_node_t *node = (vx_node_t *)n;
    if (vxIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e)
    {
        if ((callback) && (node->callback))
        {
            VX_PRINT(VX_ZONE_ERROR, "Attempting to overriding existing callback %p on Node %s!\n", node->callback, node->kernel->name);
            status = VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            node->callback = callback;
            status = VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_nodecomplete_f VX_API_CALL vxRetrieveNodeCallback(vx_node n)
{
    vx_node_t *node = (vx_node_t *)n;
    vx_nodecomplete_f cb = NULL;
    if (vxIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e)
    {
        cb = node->callback;
    }
    return cb;
}

#if defined(EXPERIMENTAL_USE_VARIANTS)
VX_API_ENTRY vx_status VX_API_CALL vxChooseKernelVariant(vx_node node, vx_char variantName [ VX_MAX_VARIANT_NAME ])
{
    /* look within the currently choosen target to find a kernel with the same
     * inner name but a different variant.
     */
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_node_t *n = (vx_node_t*)node;
    if (vxIsValidSpecificReference(&n->base, VX_TYPE_NODE) == vx_true_e)
    {
        vx_uint32 k = 0u;
        vx_target_t *target = &(n->base.context->targets[n->affinity]);

        /* Convert the kernel name to a simple name if it includes a variant. */
        vx_char *kernelName = strdup(n->kernel->name);
        vx_char *colon = strchr(kernelName, ':');
        if (colon) *colon = '\0';

        status = target->funcs.supports(target,
                                        target->name,
                                        kernelName,
                                        variantName, &k);

        free(kernelName);

        if (status == VX_SUCCESS)
        {
            /* decrementing node's internal reference to kernel before re-assignment */
            vxDecrementReference(&n->kernel->base, VX_INTERNAL);

            /* switched to new kernel */
            n->kernel = &target->kernels[k];

            /* incrementing node's internal reference to kernel after re-assignment */
            vxIncrementReference(&n->kernel->base, VX_INTERNAL);
        }
    }
    return status;
}
#endif

// ![SAMPLE EXTENSION]
vx_status vxSetChildGraphOfNode(vx_node node, vx_graph graph)
{
    vx_status status = VX_ERROR_INVALID_GRAPH;
    vx_bool valid_node = vxIsValidSpecificReference(&node->base, VX_TYPE_NODE);
    if ((valid_node == vx_true_e) && graph &&
        (vxIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e))
    {
        vx_uint32 p, num = node->kernel->signature.num_parameters;
        /* check to make sure the signature of the node matches the signature of the graph. */
        //if (node->kernel->signature.numParams == graph->numParams)
        if (graph->numParams > 0)
        {
            vx_bool match = vx_true_e;
            for (p = 0; p < num; p++)
            {
                vx_uint32 child_index = graph->parameters[p].index;
                if (graph->parameters[p].node)
                {
                    if ((node->kernel->signature.directions[p] != graph->parameters[p].node->kernel->signature.directions[child_index]) ||
                        (node->kernel->signature.states[p] != graph->parameters[p].node->kernel->signature.states[child_index]) ||
                        (node->kernel->signature.types[p] != graph->parameters[p].node->kernel->signature.types[child_index]))
                    {
                        vxAddLogEntry(&graph->base, VX_ERROR_INVALID_PARAMETERS,
                                "Graph parameter %u does not match node parameter %u\n"
                                "\t%s[%u] dir:%d req:%d type:0%x\n"
                                "\t%s[%u] dir:%d req:%d type:0%x\n",
                                p, p,
                                node->kernel->name,
                                p,
                                node->kernel->signature.directions[p],
                                node->kernel->signature.states[p],
                                node->kernel->signature.types[p],
                                graph->parameters[p].node->kernel->name,
                                p,
                                graph->parameters[p].node->kernel->signature.directions[child_index],
                                graph->parameters[p].node->kernel->signature.states[child_index],
                                graph->parameters[p].node->kernel->signature.types[child_index]);
                        match = vx_false_e;
                    }
                }
                else
                {
                    vxAddLogEntry(&graph->base, VX_ERROR_INVALID_NODE, "WARNING: the child graph has declared graph parameter index %u node reference to be NULL!\n", p);
                }
            }
            if (match == vx_true_e)
            {
                node->child = graph;
                vxIncrementReference(&graph->base, VX_INTERNAL);
                VX_PRINT(VX_ZONE_GRAPH, "Graph "VX_FMT_REF" set as child graph of "VX_FMT_REF"\n", graph, node->graph);
                status = VX_SUCCESS;
            }
        }
        else
        {
            //vxAddLogEntry((vx_reference)child, VX_ERROR_INVALID_GRAPH, "Number of parameters on graph does not equal number of parameters on node!");
            vxAddLogEntry(&graph->base, VX_ERROR_INVALID_GRAPH, "Graph must have some parameters!");
        }
    }
    else if ((valid_node == vx_true_e) && (graph == NULL) && (node->child != NULL))
    {
        vxReleaseReferenceInt((vx_reference *)&node->child, VX_TYPE_GRAPH, VX_INTERNAL, NULL);
    }
    return status;
}

vx_graph vxGetChildGraphOfNode(vx_node n)
{
    vx_node_t *node = (vx_node_t *)n;
    vx_graph_t *graph = 0;
    if (vxIsValidSpecificReference(&node->base, VX_TYPE_NODE) == vx_true_e)
    {
        graph = node->child;
    }
    return (vx_graph)graph;
}

// ![SAMPLE EXTENSION]
