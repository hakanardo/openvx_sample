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

static vx_bool read_pixel(void *base, vx_imagepatch_addressing_t *addr,
                          vx_float32 x, vx_float32 y, const vx_border_mode_t *borders, vx_uint8 *pixel)
{
    vx_bool out_of_bounds = (x < 0 || y < 0 || x >= addr->dim_x || y >= addr->dim_y);
    vx_uint32 bx, by;
    vx_uint8 *bpixel;
    if (out_of_bounds)
    {
        if (borders->mode == VX_BORDER_MODE_UNDEFINED)
            return vx_false_e;
        if (borders->mode == VX_BORDER_MODE_CONSTANT)
        {
            *pixel = borders->constant_value;
            return vx_true_e;
        }
    }

    // bounded x/y
    bx = x < 0 ? 0 : x >= addr->dim_x ? addr->dim_x - 1 : (vx_uint32)x;
    by = y < 0 ? 0 : y >= addr->dim_y ? addr->dim_y - 1 : (vx_uint32)y;

    bpixel = vxFormatImagePatchAddress2d(base, bx, by, addr);
    *pixel = *bpixel;

    return vx_true_e;
}

typedef void (*transform_f)(vx_uint32 dst_x, vx_uint32 dst_y, const vx_float32 m[], vx_float32 *src_x, vx_float32 *src_y);

static void transform_affine(vx_uint32 dst_x, vx_uint32 dst_y, const vx_float32 m[], vx_float32 *src_x, vx_float32 *src_y)
{
    *src_x = dst_x * m[0] + dst_y * m[2] + m[4];
    *src_y = dst_x * m[1] + dst_y * m[3] + m[5];
}

static void transform_perspective(vx_uint32 dst_x, vx_uint32 dst_y, const vx_float32 m[], vx_float32 *src_x, vx_float32 *src_y)
{
    vx_float32 z = dst_x * m[2] + dst_y * m[5] + m[8];

    *src_x = (dst_x * m[0] + dst_y * m[3] + m[6]) / z;
    *src_y = (dst_x * m[1] + dst_y * m[4] + m[7]) / z;
}

static vx_status vxWarpGeneric(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image,
                               const vx_border_mode_t *borders, transform_f transform)
{
    vx_status status = VX_SUCCESS;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_uint32 dst_width, dst_height;
    vx_rectangle_t src_rect;
    vx_rectangle_t dst_rect;

    vx_float32 m[9];
    vx_enum type = 0;

    vx_uint32 y = 0u, x = 0u;

    vxQueryImage(dst_image, VX_IMAGE_ATTRIBUTE_WIDTH, &dst_width, sizeof(dst_width));
    vxQueryImage(dst_image, VX_IMAGE_ATTRIBUTE_HEIGHT, &dst_height, sizeof(dst_height));

    vxGetValidRegionImage(src_image, &src_rect);
    dst_rect.start_x = 0;
    dst_rect.start_y = 0;
    dst_rect.end_x = dst_width;
    dst_rect.end_y = dst_height;

    status |= vxAccessImagePatch(src_image, &src_rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst_image, &dst_rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    status |= vxAccessMatrix(matrix, m);
    status |= vxAccessScalarValue(stype, &type);

    if (status == VX_SUCCESS)
    {
        for (y = 0u; y < dst_addr.dim_y; y++)
        {
            for (x = 0u; x < dst_addr.dim_x; x++)
            {
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);

                vx_float32 xf;
                vx_float32 yf;
                transform(x, y, m, &xf, &yf);
                xf -= (vx_float32)src_rect.start_x;
                yf -= (vx_float32)src_rect.start_y;

                if (type == VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR)
                {
                    read_pixel(src_base, &src_addr, xf, yf, borders, dst);
                }
                else if (type == VX_INTERPOLATION_TYPE_BILINEAR)
                {
                    vx_uint8 tl = 0, tr = 0, bl = 0, br = 0;
                    vx_bool defined = vx_true_e;
                    defined &= read_pixel(src_base, &src_addr, floorf(xf), floorf(yf), borders, &tl);
                    defined &= read_pixel(src_base, &src_addr, floorf(xf) + 1, floorf(yf), borders, &tr);
                    defined &= read_pixel(src_base, &src_addr, floorf(xf), floorf(yf) + 1, borders, &bl);
                    defined &= read_pixel(src_base, &src_addr, floorf(xf) + 1, floorf(yf) + 1, borders, &br);
                    if (defined)
                    {
                        vx_float32 ar = xf - floorf(xf);
                        vx_float32 ab = yf - floorf(yf);
                        vx_float32 al = 1.0f - ar;
                        vx_float32 at = 1.0f - ab;
                        *dst = tl * al * at + tr * ar * at + bl * al * ab + br * ar * ab;
                    }
                }
            }
        }

        /*! \todo compute maximum area rectangle */
    }

    status |= vxCommitMatrix(matrix, m);
    status |= vxCommitImagePatch(src_image, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst_image, &dst_rect, 0, &dst_addr, dst_base);

    return status;
}

// nodeless version of the WarpAffine kernel
vx_status vxWarpAffine(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders)
{
    return vxWarpGeneric(src_image, matrix, stype, dst_image, borders, transform_affine);
}

// nodeless version of the WarpPerspective kernel
vx_status vxWarpPerspective(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders)
{
    return vxWarpGeneric(src_image, matrix, stype, dst_image, borders, transform_perspective);
}
