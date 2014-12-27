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
 * \brief The Filter Kernel (Extras)
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>

static vx_status VX_CALLBACK vxHarrisScoreKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_FAILURE;
    if (num == 5)
    {
        vx_image grad_x = (vx_image)parameters[0];
        vx_image grad_y = (vx_image)parameters[1];
        vx_scalar sens = (vx_scalar)parameters[2];
        vx_scalar winds = (vx_scalar)parameters[3];
        vx_image dst = (vx_image)parameters[4];
        vx_float32 k = 0.0f;
        vx_uint32 block_size = 0;
        vx_rectangle_t rect;

        status = vxGetValidRegionImage(grad_x, &rect);
        status |= vxAccessScalarValue(winds, &block_size);
        status |= vxAccessScalarValue(sens, &k);
        if (status == VX_SUCCESS)
        {
            vx_int32 y, x, i, j;
            vx_border_mode_t borders = {VX_BORDER_MODE_UNDEFINED, 0};
            void *gx_base = NULL, *gy_base = NULL, *dst_base = NULL;
            vx_imagepatch_addressing_t gx_addr, gy_addr, dst_addr;

            status |= vxAccessImagePatch(grad_x, &rect, 0, &gx_addr, &gx_base, VX_READ_ONLY);
            status |= vxAccessImagePatch(grad_y, &rect, 0, &gy_addr, &gy_base, VX_READ_ONLY);
            status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
            status |= vxQueryNode(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &borders, sizeof(borders));
            /*! \todo implement other Harris Corners border modes */
            if (borders.mode == VX_BORDER_MODE_UNDEFINED)
            {
                vx_int32 b = (block_size/2) + 1;
                vx_int32 b2 = (block_size/2);
                if (status != VX_SUCCESS)
                    goto cleanup;
                vxAlterRectangle(&rect, b, b, -b, -b);
                for (y = b; (y < (vx_int32)(gx_addr.dim_y - b)); y++)
                {
                    for (x = b; x < (vx_int32)(gx_addr.dim_x - b); x++)
                    {
                        vx_int32 sum_ix2 = 0;
                        vx_int32 sum_iy2 = 0;
                        vx_int32 sum_ixy = 0;
                        vx_int64 det_A = 0;
                        vx_int64 trace_A = 0, ktrace_A2 = 0;
                        vx_int64 M_c = 0;
                        vx_float32 *pmc = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                        for (j = -b2; j <= b2; j++)
                        {
                            for (i = -b2; i <= b2; i++)
                            {
                                vx_int16 *pgx = vxFormatImagePatchAddress2d(gx_base, x+i, y+j, &gx_addr);
                                vx_int16 *pgy = vxFormatImagePatchAddress2d(gy_base, x+i, y+j, &gy_addr);
                                vx_int16 gx = *pgx;
                                vx_int16 gy = *pgy;
                                sum_ix2 += gx * gx;
                                sum_iy2 += gy * gy;
                                sum_ixy += gx * gy;
                            }
                        }
                        det_A = ((vx_int64)sum_ix2 * sum_iy2) - ((vx_int64)sum_ixy * sum_ixy);
                        trace_A = (vx_int64)sum_ix2 + sum_iy2;
                        ktrace_A2 = (vx_int64)(k * (trace_A * trace_A));
                        M_c = det_A - ktrace_A2;
                        *pmc = (vx_float32)M_c;
#if 0
                        if (sum_ix2 > 0 || sum_iy2 > 0 || sum_ixy > 0)
                        {
                            printf("Σx²=%d Σy²=%d Σxy=%d\n",sum_ix2,sum_iy2,sum_ixy);
                            printf("det|A|=%lld trace(A)=%lld M_c=%lld\n",det_A,trace_A,M_c);
                            printf("{x,y,M_c32}={%d,%d,%d}\n",x,y,M_c32);
                        }
#endif
                    }
                }
            }
            else
            {
                status = VX_ERROR_NOT_IMPLEMENTED;
            }
cleanup:
            status |= vxCommitImagePatch(grad_x, NULL, 0, &gx_addr, gx_base);
            status |= vxCommitImagePatch(grad_y, NULL, 0, &gy_addr, gy_base);
            status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);
        }
    }
    return status;
}

static vx_status VX_CALLBACK vxHarrisScoreInputValidator(vx_node node, vx_uint32 index)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if ((index == 0) || (index == 1))
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, index);

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_df_image format = 0;
            vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
            if (format == VX_DF_IMAGE_S16)
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
                if (stype == VX_TYPE_FLOAT32)
                {
                    status = VX_SUCCESS;
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
    else if (index == 3)
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
                    vx_int32 size = 0;
                    vxAccessScalarValue(scalar, &size);
                    if (size == 3 || size == 5 || size == 7)
                    {
                        status = VX_SUCCESS;
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

static vx_status VX_CALLBACK vxHarrisScoreOutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    if (index == 4)
    {
        vx_image input = 0;
        vx_parameter param = vxGetParameterByIndex(node, 0); /* we reference the input image */

        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &input, sizeof(input));
        if (input)
        {
            vx_uint32 width = 0, height = 0;
            vx_df_image format = VX_DF_IMAGE_F32;

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
    return status;
}

static vx_param_description_t harrisscore_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t harris_score_kernel = {
    VX_KERNEL_EXTRAS_HARRIS_SCORE,
    "org.khronos.extras.harris_score",
    vxHarrisScoreKernel,
    harrisscore_kernel_params, dimof(harrisscore_kernel_params),
    vxHarrisScoreInputValidator,
    vxHarrisScoreOutputValidator,
    NULL,
    NULL,
};
