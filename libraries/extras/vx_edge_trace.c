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
 * \brief Edge tracing for the Canny edge detector.
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>
#include <math.h>
#include <stdlib.h>

static const struct offset_t {
    int x;
    int y;
} dir_offsets[8] = {
    {  0, -1 },
    { -1, -1 },
    { -1,  0 },
    { -1, +1 },
    {  0, +1 },
    { +1, +1 },
    { +1,  0 },
    { +1, -1 },
};

static vx_status vxEdgeTrace(vx_image norm, vx_threshold threshold, vx_image output)
{
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t norm_addr, output_addr;
    void *norm_base = NULL, *output_base = NULL;
    vx_uint32 y = 0, x = 0;
    vx_int32 lower = 0, upper = 0;
    vx_status status = VX_SUCCESS;
    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));
    vxGetValidRegionImage(norm, &rect);

    status |= vxAccessImagePatch(norm, &rect, 0, &norm_addr, &norm_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &output_addr, &output_base, VX_WRITE_ONLY);
    if (status == VX_SUCCESS) {
        const vx_uint8 NO = 0, MAYBE = 127, YES = 255;

        /* Initially we add all YES pixels to the stack. Later we only add MAYBE
           pixels to it, and we reset their state to YES afterwards; so we can never
           add the same pixel more than once. That means that the stack size is bounded
           by the image size. */
        vx_uint32 (*tracing_stack)[2] = malloc(output_addr.dim_y * output_addr.dim_x * sizeof *tracing_stack);
        vx_uint32 (*stack_top)[2] = tracing_stack;

        for (y = 0; y < norm_addr.dim_y; y++)
            for (x = 0; x < norm_addr.dim_x; x++)
            {
                vx_uint16 *norm_ptr = vxFormatImagePatchAddress2d(norm_base, x, y, &norm_addr);
                vx_uint8 *output_ptr = vxFormatImagePatchAddress2d(output_base, x, y, &output_addr);

                if (*norm_ptr > upper)
                {
                    *output_ptr = YES;
                    (*stack_top)[0] = x;
                    (*stack_top)[1] = y;
                    ++stack_top;
                }
                else if (*norm_ptr <= lower)
                {
                    *output_ptr = NO;
                }
                else
                {
                    *output_ptr = MAYBE;
                }
            }


        while (stack_top != tracing_stack) {
            int i;
            --stack_top;
            x = (*stack_top)[0];
            y = (*stack_top)[1];

            for (i = 0; i < dimof(dir_offsets); ++i) {
                const struct offset_t offset = dir_offsets[i];
                vx_uint32 new_x, new_y;
                vx_uint8 *output_ptr;

                if (x == 0 && offset.x < 0) continue;
                if (x == output_addr.dim_x - 1 && offset.x > 0) continue;
                if (y == 0 && offset.y < 0) continue;
                if (y == output_addr.dim_y - 1 && offset.y > 0) continue;

                new_x = x + offset.x;
                new_y = y + offset.y;

                output_ptr = vxFormatImagePatchAddress2d(output_base, new_x, new_y, &output_addr);
                if (*output_ptr != MAYBE) continue;

                *output_ptr = YES;

                (*stack_top)[0] = new_x;
                (*stack_top)[1] = new_y;
                ++stack_top;
            }
        }

        free(tracing_stack);

        for (y = 0; y < output_addr.dim_y; y++)
            for (x = 0; x < output_addr.dim_x; x++)
            {
                vx_uint8 *output_ptr = vxFormatImagePatchAddress2d(output_base, x, y, &output_addr);
                if (*output_ptr == MAYBE) *output_ptr = NO;
            }

        status |= vxCommitImagePatch(norm, 0, 0, &norm_addr, norm_base);
        status |= vxCommitImagePatch(output, &rect, 0, &output_addr, output_base);
    }
    return status;
}

static vx_status VX_CALLBACK vxEdgeTraceKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 3)
    {
        vx_image norm = (vx_image)parameters[0];
        vx_threshold threshold = (vx_threshold)parameters[1];
        vx_image output = (vx_image)parameters[2];
        status = vxEdgeTrace(norm, threshold, output);
    }
    return status;
}

static vx_status VX_CALLBACK vxEdgeTraceInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    vx_parameter param = vxGetParameterByIndex(node, index);
    if (param) {
        if (index == 0)
        {
            vx_image norm = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &norm, sizeof(norm));
            if (norm)
            {
                vx_df_image format = 0;
                vxQueryImage(norm, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U16)
                    status = VX_SUCCESS;
                else
                    status = VX_ERROR_INVALID_FORMAT;
                vxReleaseImage(&norm);
            }
        }
        else if (index == 1)
        {
            vx_threshold threshold = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &threshold, sizeof(threshold));
            if (threshold)
            {
                vx_enum type = 0;
                vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_THRESHOLD_TYPE_RANGE)
                    status = VX_SUCCESS;
                else
                    status = VX_ERROR_INVALID_TYPE;
                vxReleaseThreshold(&threshold);
            }
        }
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_status VX_CALLBACK vxEdgeTraceOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter src_param = vxGetParameterByIndex(node, 0);
        if (src_param)
        {
            vx_image src = 0;
            vxQueryParameter(src_param, VX_PARAMETER_ATTRIBUTE_REF, &src, sizeof(src));
            if (src)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_U8;

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(height));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                //vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                status = VX_SUCCESS;
                vxReleaseImage(&src);
            }
            vxReleaseParameter(&src_param);
        }
    }
    return status;
}

static vx_param_description_t edge_trace_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_THRESHOLD,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t edge_trace_kernel = {
    VX_KERNEL_EXTRAS_EDGE_TRACE,
    "org.khronos.extra.edge_trace",
    vxEdgeTraceKernel,
    edge_trace_kernel_params, dimof(edge_trace_kernel_params),
    vxEdgeTraceInputValidator,
    vxEdgeTraceOutputValidator,
    NULL,
    NULL,
};
