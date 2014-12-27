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
 * \brief The Compare Object Kernels.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \defgroup group_debug_ext Debugging Extension
 */

#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>

vx_status VX_CALLBACK vxCompareImagesKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 3)
    {
        vx_image a = (vx_image)parameters[0];
        vx_image b = (vx_image)parameters[1];
        vx_scalar diffs = (vx_scalar)parameters[2];
        vx_uint32 numDiffs = 0u;
        vx_uint32 a_width, a_height;
        vx_df_image a_format;
        vx_uint32 b_width, b_height;
        vx_size a_planes, b_planes;
        vx_df_image b_format;

        vxQueryImage(a, VX_IMAGE_ATTRIBUTE_FORMAT, &a_format, sizeof(a_format));
        vxQueryImage(b, VX_IMAGE_ATTRIBUTE_FORMAT, &b_format, sizeof(b_format));
        vxQueryImage(a, VX_IMAGE_ATTRIBUTE_PLANES, &a_planes, sizeof(a_planes));
        vxQueryImage(b, VX_IMAGE_ATTRIBUTE_PLANES, &b_planes, sizeof(b_planes));
        vxQueryImage(a, VX_IMAGE_ATTRIBUTE_WIDTH,  &a_width,  sizeof(a_width));
        vxQueryImage(b, VX_IMAGE_ATTRIBUTE_WIDTH,  &b_width,  sizeof(b_width));
        vxQueryImage(a, VX_IMAGE_ATTRIBUTE_HEIGHT, &a_height, sizeof(a_height));
        vxQueryImage(b, VX_IMAGE_ATTRIBUTE_HEIGHT, &b_height, sizeof(b_height));

        if ((a_planes == b_planes) &&
            (a_format == b_format) &&
            (a_width  == b_width)  &&
            (a_height == b_height))
        {
            vx_uint32 x, y, p;
            vx_int32 i;
            void *a_base_ptrs[4] = {NULL, NULL, NULL, NULL};
            void *b_base_ptrs[4] = {NULL, NULL, NULL, NULL};
            vx_imagepatch_addressing_t a_addrs[4];
            vx_imagepatch_addressing_t b_addrs[4];
            vx_rectangle_t rect_a;
            vx_rectangle_t rect_b;
            vx_rectangle_t rect;
            vxGetValidRegionImage(a, &rect_a);
            vxGetValidRegionImage(b, &rect_b);
            if (vxFindOverlapRectangle(&rect_a, &rect_b, &rect) == vx_true_e)
            {
                status = VX_SUCCESS;
                for (p = 0; p < a_planes; p++)
                {
                    status |= vxAccessImagePatch(a, &rect, p, &a_addrs[p], &a_base_ptrs[p], VX_READ_ONLY);
                    status |= vxAccessImagePatch(b, &rect, p, &b_addrs[p], &b_base_ptrs[p], VX_READ_ONLY);
                }
                if (status != VX_SUCCESS)
                {
                    vxAddLogEntry((vx_reference)node, VX_FAILURE, "Failed to get patch on a and/or b\n");
                }
                for (p = 0; p < a_planes; p++)
                {
                    for (y = 0; y < a_addrs[p].dim_y; y+=a_addrs[p].step_y)
                    {
                        for (x = 0; x < a_addrs[p].dim_x; x+=a_addrs[p].step_x)
                        {
                            uint8_t *a_ptr = vxFormatImagePatchAddress2d(a_base_ptrs[p], x, y, &a_addrs[p]);
                            uint8_t *b_ptr = vxFormatImagePatchAddress2d(b_base_ptrs[p], x, y, &b_addrs[p]);
                            for (i = 0; i < a_addrs[p].stride_x; i++)
                            {
                                if (a_ptr[i] != b_ptr[i])
                                {
                                    numDiffs++;
                                }
                            }
                        }
                    }
                }
                vxCommitScalarValue(diffs, &numDiffs);
                for (p = 0; p < a_planes; p++)
                {
                    status |= vxCommitImagePatch(a, NULL, p, &a_addrs[p], a_base_ptrs[p]);
                    status |= vxCommitImagePatch(b, NULL, p, &b_addrs[p], b_base_ptrs[p]);
                }
            }
            if (numDiffs > 0)
            {
                vxAddLogEntry((vx_reference)node, VX_FAILURE, "%u differences found\n", numDiffs);
            }
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCompareInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        /* any width/height/format is ok */
        status = VX_SUCCESS;
    }
    if (index == 1)
    {
        vx_parameter params[] = {
            vxGetParameterByIndex(node, 0),
            vxGetParameterByIndex(node, 1),
        };
        if (params[0] && params[1])
        {
            vx_image inputs[2];

            vxQueryParameter(params[0], VX_PARAMETER_ATTRIBUTE_REF, &inputs[0], sizeof(inputs[0]));
            vxQueryParameter(params[1], VX_PARAMETER_ATTRIBUTE_REF, &inputs[1], sizeof(inputs[1]));
            if (inputs[0] && inputs[1])
            {
                vx_uint32 width[2], height[2];
                vx_df_image format[2];

                vxQueryImage(inputs[0], VX_IMAGE_ATTRIBUTE_WIDTH, &width[0], sizeof(width[0]));
                vxQueryImage(inputs[0], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[0], sizeof(height[0]));
                vxQueryImage(inputs[0], VX_IMAGE_ATTRIBUTE_FORMAT, &format[0], sizeof(format[0]));

                vxQueryImage(inputs[1], VX_IMAGE_ATTRIBUTE_WIDTH, &width[1], sizeof(width[1]));
                vxQueryImage(inputs[1], VX_IMAGE_ATTRIBUTE_HEIGHT, &height[1], sizeof(height[1]));
                vxQueryImage(inputs[1], VX_IMAGE_ATTRIBUTE_FORMAT, &format[1], sizeof(format[1]));

                if (width[0] == width[1] &&
                    height[0] == height[1] &&
                    format[0] == format[1])
                {
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&inputs[0]);
                vxReleaseImage(&inputs[1]);
            }
            vxReleaseParameter(&params[0]);
            vxReleaseParameter(&params[1]);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxCompareOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_enum stype =  VX_TYPE_UINT32;
        vxSetMetaFormatAttribute(meta, VX_SCALAR_ATTRIBUTE_TYPE, &stype, sizeof(stype));
        status = VX_SUCCESS;
    }
    return status;
}

static vx_param_description_t compare_images_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t compareimage_kernel = {
    VX_KERNEL_COMPARE_IMAGE,
    "org.khronos.debug.compare_images",
    vxCompareImagesKernel,
    compare_images_params, dimof(compare_images_params),
    vxCompareInputValidator,
    vxCompareOutputValidator,
    NULL,
    NULL,
};
