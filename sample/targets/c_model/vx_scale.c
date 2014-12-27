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
 * \brief The Image Scale Kernel
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <vx_internal.h>
#include <c_model.h>
#include <stdio.h>
#include <math.h>

/* Specification 1.0, on AREA interpolation method: "The details of this
 * sampling method are implementation defined. The implementation should
 * perform enough sampling to avoid aliasing, but there is no requirement
 * that the sample areas for adjacent output pixels be disjoint, nor that
 * the pixels be weighted evenly."
 *
 * The existing implementation of AREA can use too much heap in certain
 * circumstances, so disabling for now, and using NEAREST_NEIGHBOR instead,
 * as it also passes conformance for AREA interpolation.
 */
#define AREA_SCALE_ENABLE 0 /* TODO enable this again after changing implementation in kernels/c_model/c_scale.c */

static vx_status VX_CALLBACK vxScaleImageKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 3)
    {
        vx_image  src_image = (vx_image) parameters[0];
        vx_image  dst_image = (vx_image) parameters[1];
        vx_scalar stype     = (vx_scalar)parameters[2];
        vx_border_mode_t bordermode = {VX_BORDER_MODE_UNDEFINED, 0};
        vx_float64 *interm = NULL;
        vx_size size = 0ul;

        vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &bordermode, sizeof(bordermode));
        vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_PTR, &interm, sizeof(interm));
        vxQueryNode(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE,&size, sizeof(size));

        return vxScaleImage(src_image, dst_image, stype, &bordermode, interm, size);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxScaleImageInitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 3)
    {
        vx_image src = (vx_image)parameters[0];
        vx_image dst = (vx_image)parameters[1];
        vx_uint32 w1 = 0, h1 = 0, w2 = 0, h2 = 0;
#if AREA_SCALE_ENABLE
        vx_uint32 gcd_w = 0, gcd_h = 0;
#endif
        vx_size size = 0;

        vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &w1, sizeof(w1));
        vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &h1, sizeof(h1));
        vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_WIDTH, &w2, sizeof(w2));
        vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_HEIGHT, &h2, sizeof(h2));

/* AREA interpolation requires a scratch buffer, however, if AREA
 * implementation is disabled, then no scratch buffer is required, and
 * size can be 0 (setting to 1 so that checks can pass in the kernel) */
#if AREA_SCALE_ENABLE
        gcd_w = math_gcd(w1,w2);
        gcd_h = math_gcd(h1,h2);
        /* printf("%ux%u => %ux%u :: GCD_w %u GCD_h %u\n", w1,h1, w2,h2, gcd_w, gcd_h); */
        if (gcd_w != 0 && gcd_h != 0)
        {
            size = (w1 / gcd_w) * (w2 / gcd_w) * (h1 / gcd_h) * (h2 / gcd_h) * sizeof(vx_float64);
        }
        /* printf("Requesting "VX_FMT_SIZE" bytes for resizer\n", size); */
#else
        size = 1;
#endif
        vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_LOCAL_DATA_SIZE, &size, sizeof(size));
        status = VX_SUCCESS;
    }
    return status;
}

static vx_status VX_CALLBACK vxScaleImageInputValidator(vx_node node, vx_uint32 index)
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
    else if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar scalar = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum stype = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
                if (stype == VX_TYPE_ENUM)
                {
                    vx_enum interp = 0;
                    vxAccessScalarValue(scalar, &interp);
                    if ((interp == VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR) ||
                        (interp == VX_INTERPOLATION_TYPE_BILINEAR) ||
                        (interp == VX_INTERPOLATION_TYPE_AREA))
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

static vx_status VX_CALLBACK vxScaleImageOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);
        if (src_param && dst_param)
        {
            vx_image src = 0;
            vx_image dst = 0;
            vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
            vxQueryParameter(dst_param, VX_PARAMETER_ATTRIBUTE_REF, &dst, sizeof(dst));
            if ((src) && (dst))
            {
                vx_uint32 w1 = 0, h1 = 0, w2 = 0, h2 = 0;
                vx_df_image f1 = VX_DF_IMAGE_VIRT, f2 = VX_DF_IMAGE_VIRT;

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &w1, sizeof(w1));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &h1, sizeof(h1));
                vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_WIDTH, &w2, sizeof(w2));
                vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_HEIGHT, &h2, sizeof(h2));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &f1, sizeof(f1));
                vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_FORMAT, &f2, sizeof(f2));
                /* output can not be virtual */
                if ((w2 != 0) && (h2 != 0) && (f2 != VX_DF_IMAGE_VIRT) && (f1 == f2))
                {
                    /* fill in the meta data with the attributes so that the checker will pass */
                    ptr->type = VX_TYPE_IMAGE;
                    ptr->dim.image.format = f2;
                    ptr->dim.image.width = w2;
                    ptr->dim.image.height = h2;
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&src);
                vxReleaseImage(&dst);
            }
            vxReleaseParameter(&src_param);
            vxReleaseParameter(&dst_param);
        }
    }
    return status;
}

static vx_param_description_t scale_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

vx_kernel_description_t scale_image_kernel = {
    VX_KERNEL_SCALE_IMAGE,
    "org.khronos.openvx.scale_image",
    vxScaleImageKernel,
    scale_kernel_params, dimof(scale_kernel_params),
    vxScaleImageInputValidator,
    vxScaleImageOutputValidator,
    vxScaleImageInitializer,
    NULL,
};

static vx_status VX_CALLBACK vxHalfscaleGaussianInputValidator(vx_node node, vx_uint32 index)
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
    else if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar scalar = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum stype = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
                if (stype == VX_TYPE_INT32)
                {
                    vx_int32 ksize = 0;
                    vxAccessScalarValue(scalar, &ksize);
                    if ((ksize == 3) || (ksize == 5))
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

static vx_status VX_CALLBACK vxHalfscaleGaussianOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        vx_parameter dst_param = vxGetParameterByIndex(node, index);
        if (src_param && dst_param)
        {
            vx_image src = 0;
            vx_image dst = 0;
            vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
            vxQueryParameter(dst_param, VX_PARAMETER_ATTRIBUTE_REF, &dst, sizeof(dst));
            if ((src) && (dst))
            {
                vx_uint32 w1 = 0, h1 = 0;
                vx_df_image f1 = VX_DF_IMAGE_VIRT;

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &w1, sizeof(w1));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &h1, sizeof(h1));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &f1, sizeof(f1));

                /* fill in the meta data with the attributes so that the checker will pass */
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = f1;
                ptr->dim.image.width = (w1 + 1) / 2;
                ptr->dim.image.height = (h1 + 1) / 2;
                status = VX_SUCCESS;
            }
            if (src) vxReleaseImage(&src);
            if (dst) vxReleaseImage(&dst);
            vxReleaseParameter(&src_param);
            vxReleaseParameter(&dst_param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxHalfscaleGaussianKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 3)
    {
        vx_graph graph = vxGetChildGraphOfNode(node);
        status = vxProcessGraph(graph);
    }
    return status;
}

static const vx_uint32 gaussian5x5scale = 256;
static const vx_int16 gaussian5x5[5][5] =
{
    {1,  4,  6,  4, 1},
    {4, 16, 24, 16, 4},
    {6, 24, 36, 24, 6},
    {4, 16, 24, 16, 4},
    {1,  4,  6,  4, 1}
};

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

static vx_status VX_CALLBACK vxHalfscaleGaussianInitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 3)
    {
        vx_image input = (vx_image)parameters[0];
        vx_image output = (vx_image)parameters[1];
        vx_int32 kernel_size = 3;
        vx_convolution convolution = 0;
        vx_context context = vxGetContext((vx_reference)node);
        vx_graph graph = vxCreateGraph(context);

        if (graph)
        {
            vx_uint32 i;
            vxAccessScalarValue((vx_scalar)parameters[2], &kernel_size);
            if (kernel_size == 3 || kernel_size == 5)
            {
                if (kernel_size == 5)
                {
                    convolution = vxCreateGaussian5x5Convolution(context);
                }
                if (kernel_size == 3 || convolution)
                {
                    vx_image virt = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_U8);
                    vx_node nodes[] = {
                            kernel_size == 3 ? vxGaussian3x3Node(graph, input, virt) : vxConvolveNode(graph, input, convolution, virt),
                            vxScaleImageNode(graph, virt, output, VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR),
                    };

                    vx_border_mode_t borders;
                    vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
                    for (i = 0; i < dimof(nodes); i++) {
                        vxSetNodeAttribute(nodes[i], VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
                    }

                    status = VX_SUCCESS;
                    status |= vxAddParameterToGraphByIndex(graph, nodes[0], 0); /* input image */
                    status |= vxAddParameterToGraphByIndex(graph, nodes[1], 1); /* output image */
                    status |= vxAddParameterToGraphByIndex(graph, node, 2);     /* gradient size - refer to self to quiet sub-graph validator */
                    status |= vxVerifyGraph(graph);

                    /* release our references, the graph will hold it's own */
                    for (i = 0; i < dimof(nodes); i++) {
                        vxReleaseNode(&nodes[i]);
                    }
                    if (convolution) vxReleaseConvolution(&convolution);
                    vxReleaseImage(&virt);
                    status |= vxSetChildGraphOfNode(node, graph);
                }
            }
            vxReleaseGraph(&graph);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxHalfscaleGaussianDeinitializer(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (num == 3)
    {
        vxSetChildGraphOfNode(node, 0);
        status = VX_SUCCESS;
    }
    return status;
}

vx_kernel_description_t halfscale_gaussian_kernel = {
    VX_KERNEL_HALFSCALE_GAUSSIAN,
    "org.khronos.openvx.halfscale_gaussian",
    vxHalfscaleGaussianKernel,
    scale_kernel_params, dimof(scale_kernel_params),
    vxHalfscaleGaussianInputValidator,
    vxHalfscaleGaussianOutputValidator,
    vxHalfscaleGaussianInitializer,
    vxHalfscaleGaussianDeinitializer,
};


