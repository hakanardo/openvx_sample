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

/*!
 * \file
 * \brief The Debug Extensions Interface Library.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>
#include <stdio.h>
#include <assert.h>


//*****************************************************************************
// PUBLIC INTERFACE
//*****************************************************************************

vx_node vxCopyImageNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)output,
    };
    return vxCreateNodeByStructure(graph, VX_KERNEL_DEBUG_COPY_IMAGE, params, dimof(params));
}

vx_node vxCopyArrayNode(vx_graph graph, vx_array input, vx_array output)
{
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)output,
    };
    return vxCreateNodeByStructure(graph, VX_KERNEL_DEBUG_COPY_ARRAY, params, dimof(params));
}

vx_node vxFWriteImageNode(vx_graph graph, vx_image image, vx_char name[VX_MAX_FILE_NAME])
{
    vx_status status = VX_SUCCESS;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_array filepath = vxCreateArray(context, VX_TYPE_CHAR, VX_MAX_FILE_NAME);
    status = vxAddArrayItems(filepath, VX_MAX_FILE_NAME, &name[0], 0);
    if (filepath && status == VX_SUCCESS)
    {
        vx_reference params[] = {
            (vx_reference)image,
            (vx_reference)filepath
        };

        node = vxCreateNodeByStructure(graph, VX_KERNEL_DEBUG_FWRITE_IMAGE, params, dimof(params));
        vxReleaseArray(&filepath); /* the graph should add a reference to this, so we don't need it. */
    }
    return node;
}

vx_node vxFWriteArrayNode(vx_graph graph, vx_array arr, vx_char name[VX_MAX_FILE_NAME])
{
    vx_status status = VX_SUCCESS;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_array filepath = vxCreateArray(context, VX_TYPE_CHAR, VX_MAX_FILE_NAME);
    status = vxAddArrayItems(filepath, VX_MAX_FILE_NAME, &name[0], 0);
    if (filepath && status == VX_SUCCESS)
    {
        vx_reference params[] = {
            (vx_reference)arr,
            (vx_reference)filepath,
        };

        node = vxCreateNodeByStructure(graph, VX_KERNEL_DEBUG_FWRITE_ARRAY, params, dimof(params));
        vxReleaseArray(&filepath); // the graph should add a reference to this, so we don't need it.
    }
    return node;
}

vx_node vxFReadImageNode(vx_graph graph, vx_char name[VX_MAX_FILE_NAME], vx_image image)
{
    vx_status status = VX_SUCCESS;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_array filepath = vxCreateArray(context, VX_TYPE_CHAR, VX_MAX_FILE_NAME);
    status = vxAddArrayItems(filepath, VX_MAX_FILE_NAME, &name[0], 0);
    if (filepath && status == VX_SUCCESS)
    {
        vx_reference params[] = {
            (vx_reference)filepath,
            (vx_reference)image,
        };

        node = vxCreateNodeByStructure(graph, VX_KERNEL_DEBUG_FREAD_IMAGE, params, dimof(params));
        vxReleaseArray(&filepath); // the graph should add a reference to this, so we don't need it.
    }
    return node;
}

vx_node vxFReadArrayNode(vx_graph graph, vx_char name[VX_MAX_FILE_NAME], vx_array arr)
{
    vx_status status = VX_SUCCESS;
    vx_node node = 0;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_array filepath = vxCreateArray(context, VX_TYPE_CHAR, VX_MAX_FILE_NAME);
    status = vxAddArrayItems(filepath, VX_MAX_FILE_NAME, &name[0], 0);
    if (filepath && status == VX_SUCCESS)
    {
        vx_reference params[] = {
            (vx_reference)filepath,
            (vx_reference)arr,
        };

        node = vxCreateNodeByStructure(graph, VX_KERNEL_DEBUG_FWRITE_ARRAY, params, dimof(params));
        vxReleaseArray(&filepath); // the graph should add a reference to this, so we don't need it.
    }
    return node;
}

vx_node vxFillImageNode(vx_graph graph, vx_uint32 value, vx_image output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar fill = vxCreateScalar(context,VX_TYPE_UINT32, &value);
    vx_reference params[] = {
        (vx_reference)fill,
        (vx_reference)output,
    };
    vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_FILL_IMAGE, params, dimof(params));
    vxReleaseScalar(&fill); /* the graph will keep a reference */
    return node;
}

vx_node vxCheckImageNode(vx_graph graph, vx_image input, vx_uint32 value, vx_scalar errors)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar fill = vxCreateScalar(context,VX_TYPE_UINT32, &value);
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)fill,
        (vx_reference)errors,
    };
    vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_CHECK_IMAGE, params, dimof(params));
    vxReleaseScalar(&fill); /* the graph will keep a reference */
    return node;
}

vx_node vxCheckArrayNode(vx_graph graph, vx_array input, vx_uint8 value, vx_scalar errors)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar fill = vxCreateScalar(context,VX_TYPE_UINT8, &value);
    vx_reference params[] = {
        (vx_reference)input,
        (vx_reference)fill,
        (vx_reference)errors,
    };
    vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_CHECK_ARRAY, params, dimof(params));
    vxReleaseScalar(&fill); /* the graph will keep a reference */
    return node;
}

vx_node vxCompareImagesNode(vx_graph graph, vx_image a, vx_image b, vx_scalar diffs)
{
    vx_reference params[] = {
        (vx_reference)a,
        (vx_reference)b,
        (vx_reference)diffs,
    };
    vx_node node = vxCreateNodeByStructure(graph, VX_KERNEL_COMPARE_IMAGE, params, dimof(params));
    return node;
}

vx_node vxCopyImageFromPtrNode(vx_graph graph, void *ptr, vx_image output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar scalar = vxCreateScalar(context, VX_TYPE_SIZE, &ptr);
    vx_reference params[] = {
        (vx_reference)scalar,
        (vx_reference)output,
    };
    vx_node node = vxCreateNodeByStructure(graph,
                                           VX_KERNEL_COPY_IMAGE_FROM_PTR,
                                           params,
                                           dimof(params));
    vxReleaseScalar(&scalar);
    return node;
}

/* IMMEDIATE INTERFACES */

vx_status vxuCopyImage(vx_context context, vx_image src, vx_image dst)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxCopyImageNode(graph, src, dst);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

vx_status vxuCopyArray(vx_context context, vx_array src, vx_array dst)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxCopyArrayNode(graph, src, dst);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

vx_status vxuFReadImage(vx_context context, vx_char name[VX_MAX_FILE_NAME], vx_image image)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxFReadImageNode(graph, name, image);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

vx_status vxuFWriteImage(vx_context context, vx_image image, vx_char name[VX_MAX_FILE_NAME])
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxFWriteImageNode(graph, image, name);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

vx_status vxuFWriteArray(vx_context context, vx_array arr, vx_char name[VX_MAX_FILE_NAME])
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxFWriteArrayNode(graph, arr, name);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

vx_status vxuFillImage(vx_context context, vx_uint32 value, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxFillImageNode(graph, value, output);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

vx_status vxuCheckImage(vx_context context, vx_image input, vx_uint32 value, vx_uint32 *numErrors)
{
    vx_status status = VX_FAILURE;
    vx_scalar errs = vxCreateScalar(context, VX_TYPE_UINT32, numErrors);
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxCheckImageNode(graph, input, value, errs);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    vxCommitScalarValue(errs, numErrors);
    vxReleaseScalar(&errs);
    return status;
}


vx_status vxuCheckArray(vx_context context, vx_array input, vx_uint8 value, vx_uint32 *numErrors)
{
    vx_status status = VX_FAILURE;
    vx_scalar errs = vxCreateScalar(context, VX_TYPE_UINT32, numErrors);
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxCheckArrayNode(graph, input, value, errs);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    vxCommitScalarValue(errs, numErrors);
    vxReleaseScalar(&errs);
    return status;
}

vx_status vxuCompareImages(vx_context context, vx_image a, vx_image b, vx_uint32 *numDiffs)
{
    vx_status status = VX_FAILURE;
    vx_scalar diffs = vxCreateScalar(context, VX_TYPE_UINT32, numDiffs);
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxCompareImagesNode(graph, a, b, diffs);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    vxCommitScalarValue(diffs, numDiffs);
    vxReleaseScalar(&diffs);
    return status;
}


vx_status vxuCopyImageFromPtr(vx_context context, void *ptr, vx_image dst)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (graph)
    {
        vx_node node = vxCopyImageFromPtrNode(graph, ptr, dst);
        if (node)
        {
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = vxProcessGraph(graph);
            }
            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

