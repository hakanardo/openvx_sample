/*
 * Copyright (c) 2013-2014 The Khronos Group Inc.
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

#include <stdio.h>
#include <VX/vx_khr_tiling.h>
#include <VX/vx_helper.h>
#define _VX_TILING_EXT_INTERNAL_
#include "vx_tiling_ext.h"

/*! \file
 * \brief The tiling extension support functions file.
 * \example vx_tiling_ext.c
 */

#ifndef dimof
#define dimof(x)    (sizeof(x)/sizeof(x[0]))
#endif

static vx_status VX_CALLBACK vxFilterInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_df_image format = 0;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U8)
                {
                    status = VX_SUCCESS;
                }
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxFilterOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0); /* we reference an input image */
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_U8;

                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxReleaseImage(&input);

                status = VX_SUCCESS;
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxAddInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if ((index == 0) || (index == 1))
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_df_image format = 0;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U8)
                {
                    status = VX_SUCCESS;
                }
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxAddOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0); /* we reference an input image */
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_S16;

                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxReleaseImage(&input);

                status = VX_SUCCESS;
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxAlphaInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 0)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_df_image format = 0;
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
                if (format == VX_DF_IMAGE_U8)
                {
                    status = VX_SUCCESS;
                }
            }
            vxReleaseParameter(&param);
        }
    }
    else if (index == 1)
    {
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_scalar scalar = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &scalar, sizeof(scalar));
            if (scalar)
            {
                vx_enum type = 0;
                vxQueryScalar(scalar, VX_SCALAR_ATTRIBUTE_TYPE, &type, sizeof(type));
                if (type == VX_TYPE_FLOAT32)
                {
                    status = VX_SUCCESS;
                }
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxAlphaOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 2)
    {
        vx_parameter param = vxGetParameterByIndex(node, 0); /* we reference an input image */
        if (param)
        {
            vx_image input = 0;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
            if (input)
            {
                vx_uint32 width = 0, height = 0;
                vx_df_image format = VX_DF_IMAGE_U8;

                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                vxSetMetaFormatAttribute(meta, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

                vxReleaseImage(&input);

                status = VX_SUCCESS;
            }
            vxReleaseParameter(&param);
        }
    }
    return status;
}


/*! [publish_support] */
typedef struct _vx_tiling_kernel_t {
    /*! kernel name */
    vx_char name[VX_MAX_KERNEL_NAME];
    /*! kernel enum */
    vx_enum enumeration;
    /*! tiling flexible function pointer */
    vx_tiling_kernel_f flexible_function;
    /*! tiling fast function pointer */
    vx_tiling_kernel_f fast_function;
    /*! number of parameters */
    vx_uint32 num_params;
    /*! set of parameters */
    vx_param_description_t parameters[10];
    /*! input validator */
    vx_kernel_input_validate_f input_validator;
    /*! output validator */
    vx_kernel_output_validate_f output_validator;
    /*! block size */
    vx_tile_block_size_t block;
    /*! neighborhood around block */
    vx_neighborhood_size_t nbhd;
    /*! border information. */
    vx_border_mode_t border;
} vx_tiling_kernel_t;

static vx_tiling_kernel_t tiling_kernels[] = {
        {"org.khronos.openvx.tiling_gaussian_3x3",
          VX_KERNEL_GAUSSIAN_3x3_TILING,
          NULL,
          gaussian_image_tiling_fast,
          2,
          {{VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
           {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}},
          vxFilterInputValidator,
          vxFilterOutputValidator,
          {1, 1},
          {-1, 1,-1, 1},
          {VX_BORDER_MODE_UNDEFINED, 0},
        },
        {"org.khronos.openvx.tiling_alpha",
          VX_KERNEL_ALPHA_TILING,
          NULL,
          alpha_image_tiling,
          3,
          {{VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
           {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
           {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}},
          vxAlphaInputValidator,
          vxAlphaOutputValidator,
          {1, 1},
          {0, 0, 0, 0},
          {VX_BORDER_MODE_UNDEFINED, 0},
        },
        {"org.khronos.openvx.tiling_box_MxN",
          VX_KERNEL_BOX_MxN_TILING,
          NULL,
          box_image_tiling,
          2,
          {{VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
           {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}},
          vxFilterInputValidator,
          vxFilterOutputValidator,
          {1, 1},
          {-1, 1, -1, 1},
          {VX_BORDER_MODE_UNDEFINED, 0},
        },
        {"org.khronos.openvx.tiling_add",
          VX_KERNEL_ADD_TILING,
          NULL,
          add_image_tiling,
          3,
          {{VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
           {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
           {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}},
          vxAddInputValidator,
          vxAddOutputValidator,
          {1, 1},
          {0, 0, 0, 0},
          {VX_BORDER_MODE_UNDEFINED, 0},
        },
};
/*! [publish_support] */

/*! \brief The Entry point into a user defined kernel module */
VX_API_ENTRY vx_status VX_API_CALL vxPublishKernels(vx_context context)
{
    /*! [publish_function] */
    vx_status status = VX_SUCCESS;
    vx_uint32 k = 0;
    for (k = 0; k < dimof(tiling_kernels); k++)
    {
        vx_kernel kernel = vxAddTilingKernel(context,
                tiling_kernels[k].name,
                tiling_kernels[k].enumeration,
                tiling_kernels[k].flexible_function,
                tiling_kernels[k].fast_function,
                tiling_kernels[k].num_params,
                tiling_kernels[k].input_validator,
                tiling_kernels[k].output_validator);
        if (kernel)
        {
            vx_uint32 p = 0;
            for (p = 0; p < tiling_kernels[k].num_params; p++)
            {
                status |= vxAddParameterToKernel(kernel, p,
                            tiling_kernels[k].parameters[p].direction,
                            tiling_kernels[k].parameters[p].data_type,
                            tiling_kernels[k].parameters[p].state);
            }
            status |= vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_INPUT_NEIGHBORHOOD, &tiling_kernels[k].nbhd, sizeof(vx_neighborhood_size_t));
            status |= vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE, &tiling_kernels[k].block, sizeof(vx_tile_block_size_t));
            status |= vxSetKernelAttribute(kernel, VX_KERNEL_ATTRIBUTE_BORDER, &tiling_kernels[k].border, sizeof(vx_border_mode_t));
            if (status != VX_SUCCESS)
            {
                vxRemoveKernel(kernel);
            }
            else
            {
                status = vxFinalizeKernel(kernel);
            }
            if (status != VX_SUCCESS)
            {
                printf("Failed to publish kernel %s\n", tiling_kernels[k].name);
                break;
            }
        }
    }
    /*! [publish_function] */
    return status;
}
