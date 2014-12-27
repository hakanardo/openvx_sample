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
 * \brief The TableLookup Kernel.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */
#include <VX/vx.h>
#include <VX/vx_helper.h>

#include <omp.h>
#include <vx_internal.h>

#include <math.h>

static vx_status vxTableLookupKernel(vx_node node, vx_reference *parameters, vx_uint32 num)
{
    vx_status status  = VX_ERROR_INVALID_PARAMETERS;
    if (num == 3)
    {
        vx_image src_image = (vx_image) parameters[0];
        vx_lut   lut       = (vx_lut)parameters[1];
        vx_image dst_image = (vx_image) parameters[2];
        int chunk;
        /* int nthreads, tid; */ /* Removing nthreads and tid warning if the printf is commented out */
        vx_int32 x, y;

        void *src = NULL, *dst = NULL;
        vx_imagepatch_addressing_t src_addr;
        vx_imagepatch_addressing_t dst_addr;
        vx_rectangle_t rect;
        vx_uint8 *table = NULL, *out, *ptr;

        status  = VX_SUCCESS;
        status |= vxGetValidRegionImage(src_image, &rect);
        status |= vxAccessImagePatch(src_image, &rect, 0, &src_addr, &src, VX_READ_AND_WRITE);
        status |= vxAccessLUT(lut, (void **)&table, VX_READ_AND_WRITE);
        status |= vxAccessImagePatch(dst_image, &rect, 0, &dst_addr, &dst, VX_READ_AND_WRITE);

        if (status != VX_SUCCESS)
            return status;

#pragma omp parallel shared(y, src, dst, src_addr, dst_addr) private(/*nthreads, tid,*/ ptr, out, x)
        {
            chunk = src_addr.dim_y / TARGET_NUM_CORES;
#if 0 /* Removing nthreads and tid warning if the printf is commented out */
            tid = omp_get_thread_num();
            if (tid == 0)
            {
                nthreads = omp_get_num_threads();
                //printf("Thread[%d] chunk = %d\n", nthreads, chunk);
            }
#endif
#pragma omp for schedule(dynamic, chunk)
            for (y = 0; y < (vx_int32)src_addr.dim_y; y++)
            {
                for (x = 0; x < (vx_int32)src_addr.dim_x; x++)
                {
                    ptr = vxFormatImagePatchAddress2d(src, x, y, &src_addr);
                    out = vxFormatImagePatchAddress2d(dst, x, y, &dst_addr);
                    if (ptr && out)
                        *out = table[*ptr];
                }
            }
        }

        status |= vxCommitImagePatch(src_image, NULL, 0, &src_addr, src);
        status |= vxCommitLUT(lut, table);
        status |= vxCommitImagePatch(dst_image, &rect, 0, &dst_addr, dst);
    }
    return status;
}

static vx_status vxTableLookupInputValidator(vx_node node, vx_uint32 index)
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
            if (format == VX_DF_IMAGE_U8 || format == VX_DF_IMAGE_S16)
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
        vx_lut lut = 0;
        vxQueryParameter(param, VX_PARAMETER_ATTRIBUTE_REF, &lut, sizeof(lut));
        if (lut)
        {
            vx_enum type = 0;
            vxQueryLUT(lut, VX_LUT_ATTRIBUTE_TYPE, &type, sizeof(type));
            if (type == VX_TYPE_UINT8 || type == VX_TYPE_INT16)
            {
                status = VX_SUCCESS;
            }
            vxReleaseLUT(&lut);
        }
        vxReleaseParameter(&param);
    }
    return status;
}

static vx_status vxTableLookupOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_t *ptr)
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

                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(height));
                vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
                /* output is equal type and size */
                ptr->type = VX_TYPE_IMAGE;
                ptr->dim.image.format = VX_DF_IMAGE_U8;
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

static vx_param_description_t lut_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_LUT,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_t lut_kernel = {
    VX_KERNEL_TABLE_LOOKUP,
    "org.khronos.openvx.table_lookup",
    vxTableLookupKernel,
    lut_kernel_params, dimof(lut_kernel_params),
    vxTableLookupInputValidator,
    vxTableLookupOutputValidator,
    NULL,
    NULL,
};


