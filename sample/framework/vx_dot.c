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

#if defined(EXPERIMENTAL_USE_DOT)

#include <vx_type_pairs.h>

VX_API_ENTRY vx_status VX_API_CALL vxExportGraphToDot(vx_graph graph, vx_char dotfile[], vx_bool showData)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (vxIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        FILE *fp = fopen(dotfile, "w+");
        if (fp)
        {
            vx_uint32 n, p, n2, d;
            vx_uint32 num_next, next_nodes[VX_INT_MAX_REF];
            vx_uint32 num_last, last_nodes[VX_INT_MAX_REF];
            vx_uint32 num_left, left_nodes[VX_INT_MAX_REF];
            vx_uint32 dep_nodes[VX_INT_MAX_REF];
            vx_reference_t *data[VX_INT_MAX_REF];
            vx_uint32 num_data = 0u;

            status = VX_SUCCESS;
            fprintf(fp, "digraph {\n");
            fprintf(fp, "\tsize=4;\n");
            fprintf(fp, "\trank=LR;\n");
            fprintf(fp, "\tnode [shape=oval style=filled fillcolor=red fontsize=27];\n");
            for (n = 0; n < graph->numNodes; n++)
            {
                vx_node_t *node = graph->nodes[n];
                fprintf(fp, "\tN%u [label=\"N%u\\n%s\"];\n", n, n, node->kernel->name);
                if (showData)
                {
                    for (p = 0u; p < node->kernel->signature.num_parameters; p++)
                    {
                        if (node->parameters[p] == NULL) continue;
                        for (d = 0u; d < num_data; d++)
                        {
                            if (data[d] == node->parameters[p])
                                break;
                        }
                        if (d == num_data)
                        {
                            // new reference added to data list
                            data[num_data++] = node->parameters[p];
                        }
                    }
                }
            }
            if (showData)
            {
                for (d = 0u; d < num_data; d++)
                {
                    vx_int32 i = vxStringFromType(data[d]->type);
                    if (data[d] == NULL) continue;
                    if (data[d]->type == VX_TYPE_IMAGE)
                    {
                        vx_image_t *image = (vx_image_t *)data[d];
                        vx_char fcc[5];
                        strncpy(fcc, (char *)&image->format, 4);
                        fcc[4] = '\0';
                        fprintf(fp, "\tD%u [shape=box label=\"%ux%u\\n%4s\"];\n", d, image->width, image->height, fcc);
                    }
                    else if (data[d]->type == VX_TYPE_ARRAY)
                    {
                        vx_array_t *arr = (vx_array_t *)data[d];
                        if (arr->item_type == VX_TYPE_CHAR || arr->item_size == sizeof(char))
                            fprintf(fp, "\tD%u [shape=box label=\"\\\"%s\\\"\"];\n", d, arr->memory.ptrs[0]);
                        else
                            fprintf(fp, "\tD%u [shape=box label=\"%s\"];\n", d, type_pairs[i].name);
                    }
                    else if (data[d]->type == VX_TYPE_PYRAMID)
                    {
                        vx_pyramid_t *pyr = (vx_pyramid_t *)data[d];
                        fprintf(fp, "\tD%u [shape=triangle label=\"%lfx"VX_FMT_REF"\\nPyramid\"];\n", d, pyr->scale, pyr->levels);
                    }
                    else
                    {
                        fprintf(fp, "\tD%u [shape=box label=\"%s\"];\n", d, type_pairs[i].name);
                    }
                }
            }

            vxClearVisitation(graph);
            vxClearExecution(graph);
            memcpy(next_nodes, graph->heads, graph->numHeads * sizeof(graph->heads[0]));
            num_next = graph->numHeads;
            num_last = 0;
            num_left = 0;
            do {
                for (n = 0; n < num_next; n++)
                {
                    /* for each head, start tracing the graph */
                    vx_node_t *node = graph->nodes[next_nodes[n]];

                    if (graph->nodes[next_nodes[n]]->executed == vx_true_e) continue;

                    for (p = 0; p < node->kernel->signature.num_parameters; p++)
                    {
                        vx_uint32 count = dimof(next_nodes);

                        if (showData && node->kernel->signature.directions[p] == VX_INPUT)
                        {
                            vxFindNodesWithReference(graph,node->parameters[p],
                                                     NULL, &count,VX_OUTPUT);
                            if (count > 0) continue;
                            for (d = 0u; d < num_data; d++)
                                if (data[d] == node->parameters[p])
                                    break;
                            if (d == num_data) continue; // ref not found
                            fprintf(fp, "\tD%u -> N%u;\n", d, next_nodes[n]);
                        }
                        else if (node->kernel->signature.directions[p] == VX_OUTPUT)
                        {
                            status = vxFindNodesWithReference(graph,
                                                              node->parameters[p],
                                                              dep_nodes,
                                                              &count,
                                                              VX_INPUT);
                            //printf("N%u has %u dep nodes on parameter[%u], %d\n", next_nodes[n], count, p, status);
                            for (n2 = 0; status == VX_SUCCESS && n2 < count; n2++)
                            {
                                if (showData)
                                {
                                    for (d = 0u; d < num_data; d++)
                                        if (data[d] == node->parameters[p])
                                            break;
                                    fprintf(fp, "\tN%u -> D%u -> N%u;\n", next_nodes[n], d, dep_nodes[n2]);
                                }
                                else
                                {
                                    fprintf(fp, "\tN%u -> N%u;\n", next_nodes[n], dep_nodes[n2]);
                                }
                            }
                        }
                    }
                }
                memcpy(last_nodes, next_nodes, num_next * sizeof(next_nodes[0]));
                num_last = num_next;
                num_next = 0;
                vxFindNextNodes(graph, last_nodes, num_last, next_nodes, &num_next, left_nodes, &num_left);
            } while (num_next > 0);
            vxClearVisitation(graph);
            vxClearExecution(graph);
            fprintf(fp, "}\n");
            fclose(fp);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to open file for writing!\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Not a graph!\n");
    }
    return status;
}

#endif
