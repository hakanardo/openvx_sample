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

#include <c_model.h>

typedef vx_int32 (arithmeticOp)(vx_int32, vx_int32);

static vx_int32 vx_add_op(vx_int32 a, vx_int32 b)
{
    return (a + b);
}

static vx_int32 vx_sub_op(vx_int32 a, vx_int32 b)
{
    return (a - b);
}

// generic arithmetic op
static vx_status vxBinaryU8S16OverflowOp(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output, arithmeticOp op)
{
    vx_enum overflow_policy = -1;
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base   = NULL;
    void *src_base[2] = {NULL, NULL};
    vx_imagepatch_addressing_t dst_addr, src_addr[2];
    vx_rectangle_t rect;
    vx_df_image in0_format = 0;
    vx_df_image in1_format = 0;
    vx_df_image out_format = 0;
    vx_status status = VX_SUCCESS;

    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &out_format, sizeof(out_format));
    vxQueryImage(in0, VX_IMAGE_ATTRIBUTE_FORMAT, &in0_format, sizeof(in0_format));
    vxQueryImage(in1, VX_IMAGE_ATTRIBUTE_FORMAT, &in1_format, sizeof(in1_format));

    status = vxGetValidRegionImage(in0, &rect);
    status |= vxAccessImagePatch(in0, &rect, 0, &src_addr[0], (void **)&src_base[0], VX_READ_ONLY);
    status |= vxAccessImagePatch(in1, &rect, 0, &src_addr[1], (void **)&src_base[1], VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, (void **)&dst_base, VX_WRITE_ONLY);
    status |= vxAccessScalarValue(policy_param, &overflow_policy);
    width = src_addr[0].dim_x;
    height = src_addr[0].dim_y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            /* Either image may be U8 or S16. */
            void *src0p = vxFormatImagePatchAddress2d(src_base[0], x, y, &src_addr[0]);
            void *src1p = vxFormatImagePatchAddress2d(src_base[1], x, y, &src_addr[1]);
            void *dstp = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);

            /* Convert inputs to int32, perform the operation. */
            vx_int32 src0 = in0_format == VX_DF_IMAGE_U8 ? *(vx_uint8 *)src0p : *(vx_int16 *)src0p;
            vx_int32 src1 = in1_format == VX_DF_IMAGE_U8 ? *(vx_uint8 *)src1p : *(vx_int16 *)src1p;
            vx_int32 overflowing_result = op(src0, src1);
            vx_int32 final_result_value;

            /* Finally, overflow-check as per the target type and policy. */
            if (overflow_policy == VX_CONVERT_POLICY_SATURATE)
            {
                if (out_format == VX_DF_IMAGE_U8)
                {
                    if (overflowing_result > UINT8_MAX)
                        final_result_value = UINT8_MAX;
                    else if (overflowing_result < 0)
                        final_result_value = 0;
                    else
                        final_result_value = overflowing_result;
                }
                else /* Already verified to be VX_DF_IMAGE_S16 */
                {
                    if (overflowing_result > INT16_MAX)
                        final_result_value = INT16_MAX;
                    else if (overflowing_result < INT16_MIN)
                        final_result_value = INT16_MIN;
                    else
                        final_result_value = overflowing_result;
                }
            }
            else /* Already verified to be VX_CONVERT_POLICY_WRAP. */
            {
                /* Just for show: the final assignment will wrap too. */
                final_result_value = (out_format == VX_DF_IMAGE_U8) ?
                    (vx_uint8)overflowing_result : (vx_int16)overflowing_result;
            }

            if (out_format == VX_DF_IMAGE_U8)
                *(vx_uint8 *)dstp = (vx_uint8)final_result_value;
            else
                *(vx_int16 *)dstp = (vx_int16)final_result_value;
        }
    }
    status |= vxCommitImagePatch(in0, NULL, 0, &src_addr[0], src_base[0]);
    status |= vxCommitImagePatch(in1, NULL, 0, &src_addr[1], src_base[1]);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);

    return status;
}

// nodeless version of the Addition kernel
vx_status vxAddition(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    return vxBinaryU8S16OverflowOp(in0, in1, policy_param, output, vx_add_op);
}

// nodeless version of the Subtraction kernel
vx_status vxSubtraction(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    return vxBinaryU8S16OverflowOp(in0, in1, policy_param, output, vx_sub_op);
}

