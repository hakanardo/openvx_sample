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
 * \brief The Filter Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vxu.h>
#include <VX/vx_helper.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_lib_debug.h>
#include <vx_internal.h>

static vx_param_description_t harris_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, // strength_thresh
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, // min_distance
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, // sensitivity
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, // gradient size
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}, // block size
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

static vx_status VX_CALLBACK vxHarrisCornersKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == dimof(harris_kernel_params))
    {
        vx_graph graph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(graph);
    }
    return status;
}

static vx_status VX_CALLBACK vxHarrisInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image input = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if ((status == VX_SUCCESS) && (input))
            {
                vx_df_image format = 0;
                status = vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if ((status == VX_SUCCESS) && (format == VX_DF_IMAGE_U8))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&input);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar sens = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
            if ((status == VX_SUCCESS) && (sens))
            {
                vx_enum type = 0;
                vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32)
                {
                    status = VX_SUCCESS;
                }
                else
                {
                    status = VX_ERROR_INVALID_TYPE;
                }
                vxReleaseScalar(&sens);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar sens = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
            if ((status == VX_SUCCESS) && (sens))
            {
                vx_enum type = 0;
                vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32)
                {
                    vx_float32 d = 0.0f;
                    status = vxAccessScalarValue(sens, &d);
                    if ((status == VX_SUCCESS) && (1.0 <= d) && (d <= 5.0))
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
                vxReleaseScalar(&sens);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 3)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar sens = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &sens, sizeof(sens));
            if ((status == VX_SUCCESS) && (sens))
            {
                vx_enum type = 0;
                vxQueryScalar(sens, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32)
                {
                    vx_float32 k = 0.0f;
                    vxAccessScalarValue(sens, &k);
                    VX_PRINT(VX_ZONE_INFO, "k = %lf\n", k);
                    if ((0.040000f <= k) && (k < 0.150001f))
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
                vxReleaseScalar(&sens);
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 4 || index == 5)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar scalar = 0;
            status = vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if ((status == VX_SUCCESS) && (scalar))
            {
                vx_enum type = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_INT32)
                {
                    vx_int32 size = 0;
                    vxAccessScalarValue(scalar, &size);
                    VX_PRINT(VX_ZONE_INFO, "size = %u\n", size);
                    if ((size == 3) || (size == 5) || (size == 7))
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
                vxReleaseScalar(&scalar);
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxHarrisOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 6)
    {
        ptr->type = VX_TYPE_ARRAY;
        ptr->dim.array.item_type = VX_TYPE_KEYPOINT;
        ptr->dim.array.capacity = 0; /* no defined capacity requirement */
        status = VX_SUCCESS;
    }
    else if (index == 7)
    {
        ptr->dim.scalar.type = VX_TYPE_UINT32;
        status = VX_SUCCESS;
    }
    return status;
}

static vx_status VX_CALLBACK vxHarrisInitializer(vx_node node, vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == dimof(harris_kernel_params))
    {
        vx_image src = (vx_image)parameters[0];
        vx_scalar str = (vx_scalar)parameters[1];
        vx_scalar min = (vx_scalar)parameters[2];
        vx_scalar sen = (vx_scalar)parameters[3];
        vx_scalar win = (vx_scalar)parameters[4];
        vx_scalar blk = (vx_scalar)parameters[5];
        vx_array arr = (vx_array)parameters[6];
        vx_scalar num_corners = (vx_scalar)parameters[7];
        vx_context c = vxGetContext((vx_reference)node);
        vx_graph g = vxCreateGraph(c);
        vxLoadKernels(c, "openvx-extras");
        vxLoadKernels(c, "openvx-debug");
        if (g)
        {
            vx_uint32 i = 0;
            vx_int32 ds = 4;
            vx_scalar shift = vxCreateScalar(c, VX_TYPE_INT32, &ds);
            vx_image virts[] = {
                    vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // Gx
                    vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // Gy
                    vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // Score
                    vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_VIRT), // Suppressed
                    vxCreateVirtualImage(g, 0, 0, VX_DF_IMAGE_U8), // Shifted Suppressed Log10
            };
            vx_node nodes[] = {
                    vxSobelMxNNode(g, src, win, virts[0], virts[1]),
                    vxHarrisScoreNode(g, virts[0], virts[1], sen, blk, virts[2]),
                    vxEuclideanNonMaxNode(g, virts[2], str, min, virts[3]),
                    vxImageListerNode(g, virts[3], arr, num_corners),
#if defined(OPENVX_DEBUGGING)
                    vxConvertDepthNode(g,virts[3],virts[4],VX_CONVERT_POLICY_WRAP,shift),
                    vxFWriteImageNode(g,virts[4],"oharris_strength_power_up4.pgm"),
#endif

            };
            status = VX_SUCCESS;
            status |= vxAddParameterToGraphByIndex(g, nodes[0], 0); // src
            status |= vxAddParameterToGraphByIndex(g, nodes[2], 1); // str
            status |= vxAddParameterToGraphByIndex(g, nodes[2], 2); // min
            status |= vxAddParameterToGraphByIndex(g, nodes[1], 2); // sen
            status |= vxAddParameterToGraphByIndex(g, nodes[0], 1); // win
            status |= vxAddParameterToGraphByIndex(g, nodes[1], 3); // blk
            status |= vxAddParameterToGraphByIndex(g, nodes[3], 1); // arr
            status |= vxAddParameterToGraphByIndex(g, nodes[3], 2); // num_corners
            for (i = 0; i < dimof(nodes); i++)
            {
                vxReleaseNode(&nodes[i]);
            }
            for (i = 0; i < dimof(virts); i++)
            {
                vxReleaseImage(&virts[i]);
            }
            vxReleaseScalar(&shift);
            status |= vxVerifyGraph(g);
            VX_PRINT(VX_ZONE_INFO, "Status from Child Graph = %d\n", status);
            if (status == VX_SUCCESS)
            {
                status = vxSetChildGraphOfNode(node, g);
            }
            else
            {
                vxReleaseGraph(&g);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxHarrisDeinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == dimof(harris_kernel_params))
    {
        vx_graph graph = vxGetChildGraphOfNode(node);
        vxReleaseGraph(&graph);
        /* set graph to "null" */
        vxSetChildGraphOfNode(node, 0);
        status = VX_SUCCESS;
    }
    return status;
}

vx_kernel_description_t harris_kernel = {
    VX_KERNEL_HARRIS_CORNERS,
    "org.khronos.openvx.harris_corners",
    vxHarrisCornersKernel,
    harris_kernel_params, dimof(harris_kernel_params),
    vxHarrisInputValidator,
    vxHarrisOutputValidator,
    vxHarrisInitializer,
    vxHarrisDeinitializer,
};
