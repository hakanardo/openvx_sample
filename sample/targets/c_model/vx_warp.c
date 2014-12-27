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
 * \brief The Warp Affine and Perspective Kernels
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_helper.h>
#include <vx_internal.h>
#include <c_model.h>

static vx_status VX_CALLBACK vxWarpPerspectiveKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 4)
    {
        vx_image  src_image = (vx_image) parameters[0];
        vx_matrix matrix    = (vx_matrix)parameters[1];
        vx_scalar stype     = (vx_scalar)parameters[2];
        vx_image  dst_image = (vx_image) parameters[3];

        vx_border_mode_t borders;
        vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

        return vxWarpPerspective(src_image, matrix, stype, dst_image, &borders);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status VX_CALLBACK vxWarpAffineKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    if (num == 4)
    {
        vx_image  src_image = (vx_image) parameters[0];
        vx_matrix matrix    = (vx_matrix)parameters[1];
        vx_scalar stype     = (vx_scalar)parameters[2];
        vx_image  dst_image = (vx_image) parameters[3];

        vx_border_mode_t borders;
        vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));

        return vxWarpAffine(src_image, matrix, stype, dst_image, &borders);
    }
    return VX_ERROR_INVALID_PARAMETERS;
}

static vx_status vxWarpInputValidator(vx_node node, vx_uint32 index, vx_size mat_columns)
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
        vx_parameter param = vxGetParameterByIndex(node, index);
        if (param)
        {
            vx_matrix matrix;
            vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &matrix, sizeof(matrix));
            if (matrix)
            {
                vx_enum data_type = 0;
                vx_size rows = 0ul, columns = 0ul;
                vxQueryMatrix(matrix, VX_MATRIX_ATTRIBUTE_TYPE, &data_type, sizeof(data_type));
                vxQueryMatrix(matrix, VX_MATRIX_ATTRIBUTE_ROWS, &rows, sizeof(rows));
                vxQueryMatrix(matrix, VX_MATRIX_ATTRIBUTE_COLUMNS, &columns, sizeof(columns));
                if ((data_type == VX_TYPE_FLOAT32) && (columns == mat_columns) && (rows == 3))
                {
                    status = VX_SUCCESS;
                }
                vxReleaseMatrix(&matrix);
            }
            vxReleaseParameter(&param);
        }
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
                        (interp == VX_INTERPOLATION_TYPE_BILINEAR))
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

static vx_status VX_CALLBACK vxWarpAffineInputValidator(vx_node node, vx_uint32 index)
{
    return vxWarpInputValidator(node, index, 2);
}

static vx_status VX_CALLBACK vxWarpPerspectiveInputValidator(vx_node node, vx_uint32 index)
{
    return vxWarpInputValidator(node, index, 3);
}

static vx_status VX_CALLBACK vxWarpOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 3)
    {
        vx_parameter dst_param = vxGetParameterByIndex(node, index);
        if (dst_param)
        {
            vx_image dst = 0;
            vxQueryParameter(dst_param, VX_PARAMETER_ATTRIBUTE_REF, &dst, sizeof(dst));
            if (dst)
            {
                vx_uint32 w1 = 0, h1 = 0;
                vx_df_image f1 = VX_DF_IMAGE_VIRT;

                vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_WIDTH, &w1, sizeof(w1));
                vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_HEIGHT, &h1, sizeof(h1));
                vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_FORMAT, &f1, sizeof(f1));
                /* output can not be virtual */
                if ((w1 != 0) && (h1 != 0) && (f1 == VX_DF_IMAGE_U8))
                {
                    /* fill in the meta data with the attributes so that the checker will pass */
                    ptr->type = VX_TYPE_IMAGE;
                    ptr->dim.image.format = VX_DF_IMAGE_U8;
                    ptr->dim.image.width = w1;
                    ptr->dim.image.height = h1;
                    status = VX_SUCCESS;
                }
                vxReleaseImage(&dst);
            }
            vxReleaseParameter(&dst_param);
        }
    }
    return status;
}

static vx_param_description_t warp_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_MATRIX, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t warp_affine_kernel = {
    VX_KERNEL_WARP_AFFINE,
    "org.khronos.openvx.warp_affine",
    vxWarpAffineKernel,
    warp_kernel_params, dimof(warp_kernel_params),
    vxWarpAffineInputValidator,
    vxWarpOutputValidator,
    NULL,
    NULL,
};

vx_kernel_description_t warp_perspective_kernel = {
    VX_KERNEL_WARP_PERSPECTIVE,
    "org.khronos.openvx.warp_perspective",
    vxWarpPerspectiveKernel,
    warp_kernel_params, dimof(warp_kernel_params),
    vxWarpPerspectiveInputValidator,
    vxWarpOutputValidator,
    NULL,
    NULL,
};
