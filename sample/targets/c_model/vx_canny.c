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
 * \brief The Canny Edge Detector Kernel Implementation.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <math.h>

/*! \note Look at \ref vxCannyEdgeDetectorNode to see how this pyramid construction works */

static vx_status VX_CALLBACK vxCannyEdgeKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 5)
    {
        vx_graph graph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(graph);
    }
    return status;
}

static vx_status VX_CALLBACK vxCannyEdgeInitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 5)
    {
        vx_image input = (vx_image)parameters[0];
        vx_threshold hyst = (vx_threshold)parameters[1];
        vx_scalar gradient_size = (vx_scalar)parameters[2];
        vx_scalar norm_type = (vx_scalar)parameters[3];
        vx_image output = (vx_image)parameters[4];

        vx_context context = vxGetContext((vx_reference)node);
        vx_graph graph = 0;
        status = vxLoadKernels(context, "openvx-extras");
        if (status != VX_SUCCESS)
        {
            return status;
        }
        graph = vxCreateGraph(context);
        if (graph)
        {
            vx_uint32 i;
            vx_image virts[] = {
                vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),  /* 0: Gx */
                vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),  /* 1: Gy */
                vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),  /* 2: Mag */
                vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),  /* 3: Phase */
                vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT),  /* 4: Nonmax */
            };

            vx_node nodes[] = {
                vxSobelMxNNode(graph, input, gradient_size, virts[0], virts[1]),
                vxElementwiseNormNode(graph, virts[0], virts[1], norm_type, virts[2]),
                vxPhaseNode(graph, virts[0], virts[1], virts[3]),
                vxNonMaxSuppressionNode(graph, virts[2], virts[3], virts[4]),
                vxEdgeTraceNode(graph, virts[4], hyst, output),
            };

            vx_border_mode_t borders;
            vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
            for (i = 0; i < dimof(nodes); i++) {
                vxSetNodeAttribute(nodes[i], VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
            }

            status |= vxAddParameterToGraphByIndex(graph, nodes[0], 0); /* input image */
            status |= vxAddParameterToGraphByIndex(graph, nodes[4], 1); /* threshold */
            status |= vxAddParameterToGraphByIndex(graph, nodes[0], 1); /* gradient size */
            status |= vxAddParameterToGraphByIndex(graph, nodes[1], 2); /* norm type */
            status |= vxAddParameterToGraphByIndex(graph, nodes[4], 2); /* output image */
            status |= vxVerifyGraph(graph);
            /* release our references, the graph will hold it's own */
            for (i = 0; i < dimof(nodes); i++) {
                vxReleaseNode(&nodes[i]);
            }
            for (i = 0; i < dimof(virts); i++) {
                vxReleaseImage(&virts[i]);
            }
            status = vxSetChildGraphOfNode(node, graph);
            vxReleaseGraph(&graph);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCannyEdgeDeinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 5)
    {
        vxSetChildGraphOfNode(node, 0);
        status = VX_SUCCESS;
    }
    return status;
}

static vx_status VX_CALLBACK vxCannyEdgeInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_U8)
            {
                status = VX_SUCCESS;
            }
            vxReleaseImage(&input);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 1)
    {
        vx_threshold hyst = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &hyst, sizeof(hyst));
        if (hyst)
        {
            vx_enum type = 0;
            vxQueryThreshold(hyst, VX_THRESHOLD_ATTRIBUTE_TYPE, &type, sizeof(type));
            if (type == VX_THRESHOLD_TYPE_RANGE)
            {
                status = VX_SUCCESS;
            }
            vxReleaseThreshold(&hyst);
        }
        vxReleaseParameter(&param);
    }
    else if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar value = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &value, sizeof(value));
            if (value)
            {
                vx_enum stype = 0;
                vxQueryScalar(value, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
                if (stype == VX_TYPE_INT32)
                {
                    vx_int32 gs = 0;
                    vxAccessScalarValue(value, &gs);
                    if ((gs == 3) || (gs == 5) || (gs == 7))
                    {
                        status = VX_SUCCESS;
                    }
                    else
                    {
                        status = VX_ERROR_INVALID_VALUE;
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_TYPE;
                }
                vxReleaseScalar(&value);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 3)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar value = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &value, sizeof(value));
            if (value)
            {
                vx_enum norm = 0;
                vxAccessScalarValue(value, &norm);
                if ((norm == VX_NORM_L1) ||
                    (norm == VX_NORM_L2))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseScalar(&value);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCannyEdgeOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 4)
    {
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        if (src_param)
        {
            vx_image src = 0;
            vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
            if (src)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_VIRT;

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(height));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                /* fill in the meta data with the attributes so that the checker will pass */
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = format;
                ptr->dim.image.width = width;
                ptr->dim.image.height = height;
                status = VX_SUCCESS;
                vxReleaseImage(&src);
            }
            vxReleaseParameter(&src_param);
        }
    }
    return status;
}

static vx_param_description_t canny_kernel_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t canny_kernel = {
    VX_KERNEL_CANNY_EDGE_DETECTOR,
    "org.khronos.openvx.canny_edge_detector",
    vxCannyEdgeKernel,
    canny_kernel_params, dimof(canny_kernel_params),
    vxCannyEdgeInputValidator,
    vxCannyEdgeOutputValidator,
    vxCannyEdgeInitializer,
    vxCannyEdgeDeinitializer,
};
