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
 * \brief The Image Pyramid Kernel
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <math.h>


static const vx_uint32 gaussian5x5scale = 256;
static const vx_int16 gaussian5x5[5][5] =
{
    {1,  4,  6,  4, 1},
    {4, 16, 24, 16, 4},
    {6, 24, 36, 24, 6},
    {4, 16, 24, 16, 4},
    {1,  4,  6,  4, 1}
};

static vx_param_description_t pyramid_kernel_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
};

/*! \note Look at \ref vxPyramidNode to see how this pyramid construction works */

static vx_status VX_CALLBACK vxPyramidKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == dimof(pyramid_kernel_params))
    {
        vx_graph graph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(graph);
    }
    return status;
}

static vx_convolution vxCreateGaussian5x5Convolution(vx_context context)
{
    vx_convolution conv = vxCreateConvolution(context, 5, 5);
    vx_status status = vxAccessConvolutionCoefficients(conv, NULL);
    if (status != VX_SUCCESS)
    {
        vxReleaseConvolution(&conv);
        return NULL;
    }
    status = vxCommitConvolutionCoefficients(conv, (vx_int16 *)gaussian5x5);
    if (status != VX_SUCCESS)
    {
        vxReleaseConvolution(&conv);
        return NULL;
    }

    vxSetConvolutionAttribute(conv, VX_CONVOLUTION_ATTRIBUTE_SCALE, (void *)&gaussian5x5scale, sizeof(vx_uint32));
    if (status != VX_SUCCESS)
    {
        vxReleaseConvolution(&conv);
        return NULL;
    }
    return conv;
}

static vx_status VX_CALLBACK vxPyramidInitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 2)
    {
        vx_size lev, levels = 1;
        vx_border_mode_t border;
        vx_image input = (vx_image)parameters[0];
        vx_pyramid gaussian = (vx_pyramid)parameters[1];
        vx_context context = vxGetContext((vx_reference)node);
        vx_graph graph = vxCreateGraph(context);
        vx_enum interp = VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR;
        status = vxLoadKernels(context, "openvx-debug");
        if (status != VX_SUCCESS)
        {
            return status;
        }

        status = vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
        if (status != VX_SUCCESS)
        {
            return status;
        }
        if (graph)
        {
            vx_image level0 = vxGetPyramidLevel(gaussian, 0);
            vx_node copy = vxCopyImageNode(graph, input, level0);
            vxReleaseImage(&level0);
            status |= vxQueryPyramid(gaussian, VX_PYRAMID_ATTRIBUTE_LEVELS, &levels, sizeof(levels));
            for (lev = 1; lev < levels; lev++)
            {
                /*! \internal Scaling is already precomputed for each level by the \ref vxCreatePyramid function */
                vx_image tmp0 = (lev-1==0 ? input : vxGetPyramidLevel(gaussian, (vx_uint32)lev-1));
                vx_image tmp1 = vxGetPyramidLevel(gaussian, (vx_uint32)lev);
                vx_image virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8);
                vx_convolution conv = vxCreateGaussian5x5Convolution(context);
                vx_node  gtmp = vxConvolveNode(graph, tmp0, conv, virt);
                vx_node  stmp = vxScaleImageNode(graph, virt, tmp1, interp);

                vxSetNodeAttribute(gtmp, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
                /* decrements the references */
                if (lev > 1) vxReleaseImage(&tmp0);
                vxReleaseConvolution(&conv);
                vxReleaseImage(&tmp1);
                vxReleaseImage(&virt);
                vxReleaseNode(&gtmp);
                vxReleaseNode(&stmp);
            }
            vxReleaseNode(&copy);
            status = vxVerifyGraph(graph);
            if (status == VX_SUCCESS)
            {
                status = VX_SUCCESS;
                status |= vxAddParameterToGraphByIndex(graph, node, 0); // img
                status |= vxAddParameterToGraphByIndex(graph, node, 1); // pyr
                status = vxSetChildGraphOfNode(node, graph);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxPyramidDeinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == dimof(pyramid_kernel_params))
    {
        vx_graph graph = vxGetChildGraphOfNode(node);
        vxReleaseGraph(&graph);
        /* set graph to "null" */
        vxSetChildGraphOfNode(node, 0);
        status = VX_SUCCESS;
    }
    return status;
}

static vx_status VX_CALLBACK vxPyramidInputValidator(vx_node node, vx_uint32 index)
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
    return status;
}

static vx_status VX_CALLBACK vxPyramidOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_image src = 0;
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);

        vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
        if (src)
        {
            vx_pyramid dst = 0;
            vxQueryParameter(dst_param, VX_PARAMETER_ATTRIBUTE_REF, &dst, sizeof(dst));

            if (dst)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format;
                vx_size num_levels;
                vx_float32 scale;

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                vxQueryPyramid(dst, VX_PYRAMID_ATTRIBUTE_LEVELS, &num_levels, sizeof(num_levels));
                vxQueryPyramid(dst, VX_PYRAMID_ATTRIBUTE_SCALE, &scale, sizeof(scale));

                /* fill in the meta data with the attributes so that the checker will pass */
                ptr->type = VX_TYPE_PYRAMID;
                ptr->dim.pyramid.width = width;
                ptr->dim.pyramid.height = height;
                ptr->dim.pyramid.format = format;
                ptr->dim.pyramid.levels = num_levels;
                ptr->dim.pyramid.scale = scale;
                status = VX_SUCCESS;
                vxReleasePyramid(&dst);
            }
            vxReleaseImage(&src);
        }
        vxReleaseParameter(&dst_param);
        vxReleaseParameter(&src_param);
    }
    return status;
}



vx_kernel_description_t pyramid_kernel = {
    VX_KERNEL_GAUSSIAN_PYRAMID,
    "org.khronos.openvx.pyramid",
    vxPyramidKernel,
    pyramid_kernel_params, dimof(pyramid_kernel_params),
    vxPyramidInputValidator,
    vxPyramidOutputValidator,
    vxPyramidInitializer,
    vxPyramidDeinitializer,
};


