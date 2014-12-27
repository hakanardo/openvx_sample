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

typedef vx_uint8 (bitwiseOp)(vx_uint8, vx_uint8);

static vx_uint8 vx_and_op(vx_uint8 a, vx_uint8 b)
{
    return a & b;
}

static vx_uint8 vx_or_op(vx_uint8 a, vx_uint8 b)
{
    return a | b;
}

static vx_uint8 vx_xor_op(vx_uint8 a, vx_uint8 b)
{
    return a ^ b;
}

// generic bitwise op
static vx_status vxBinaryU8Op(vx_image in1, vx_image in2, vx_image output, bitwiseOp op)
{
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base   = NULL;
    void *src_base[2] = {NULL, NULL};
    vx_imagepatch_addressing_t dst_addr, src_addr[2];
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;

    status = vxGetValidRegionImage(in1, &rect);
    status |= vxAccessImagePatch(in1, &rect, 0, &src_addr[0], (void **)&src_base[0], VX_READ_ONLY);
    status |= vxAccessImagePatch(in2, &rect, 0, &src_addr[1], (void **)&src_base[1], VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, (void **)&dst_base, VX_WRITE_ONLY);
    width = src_addr[0].dim_x;
    height = src_addr[0].dim_y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            vx_uint8 *src[2] = {
                vxFormatImagePatchAddress2d(src_base[0], x, y, &src_addr[0]),
                vxFormatImagePatchAddress2d(src_base[1], x, y, &src_addr[1]),
            };
            vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);

            *dst = op(*src[0], *src[1]);
        }
    }
    status |= vxCommitImagePatch(in1, NULL, 0, &src_addr[0], src_base[0]);
    status |= vxCommitImagePatch(in2, NULL, 0, &src_addr[1], src_base[1]);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);

    return status;
}

// nodeless version of the And kernel
vx_status vxAnd(vx_image in1, vx_image in2, vx_image output)
{
    return vxBinaryU8Op(in1, in2, output, vx_and_op);
}

// nodeless version of the Or kernel
vx_status vxOr(vx_image in1, vx_image in2, vx_image output)
{
    return vxBinaryU8Op(in1, in2, output, vx_or_op);
}

// nodeless version of the And kernel
vx_status vxXor(vx_image in1, vx_image in2, vx_image output)
{
    return vxBinaryU8Op(in1, in2, output, vx_xor_op);
}

// nodeless version of the Not kernel
vx_status vxNot(vx_image input, vx_image output)
{
    vx_uint32 y, x, width = 0, height = 0;
    void *dst_base = NULL;
    void *src_base = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr;
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;

    status = vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &src_addr, (void **)&src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, (void **)&dst_base, VX_WRITE_ONLY);
    height = src_addr.dim_y;
    width = src_addr.dim_x;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            vx_uint8 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);

            *dst = ~*src;
        }
    }
    status |= vxCommitImagePatch(input, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);

    return status;
}

