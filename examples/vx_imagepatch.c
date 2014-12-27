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
#include <VX/vx.h>

#define PATCH_DIM 16

vx_status example_imagepatch(vx_context context)
{
    //! [imagepatch]
    vx_status status = VX_SUCCESS;
    void *base_ptr = NULL;
    vx_uint32 width = 640, height = 480, plane = 0;
    vx_image image = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t addr;

    rect.start_x = rect.start_y = 0;
    rect.end_x = rect.end_y = PATCH_DIM;

    status = vxAccessImagePatch(image, &rect, plane,
                                &addr, &base_ptr,
                                VX_READ_AND_WRITE);
    if (status == VX_SUCCESS)
    {
        vx_uint32 x,y,i,j;
        vx_uint8 pixel = 0;

        /* a couple addressing options */

        /* use linear addressing function/macro */
        for (i = 0; i < addr.dim_x*addr.dim_y; i++) {
            vx_uint8 *ptr2 = vxFormatImagePatchAddress1d(base_ptr,
                                                         i, &addr);
            *ptr2 = pixel;
        }

        /* 2d addressing option */
        for (y = 0; y < addr.dim_y; y+=addr.step_y) {
            for (x = 0; x < addr.dim_x; x+=addr.step_x) {
                vx_uint8 *ptr2 = vxFormatImagePatchAddress2d(base_ptr,
                                                             x, y, &addr);
                *ptr2 = pixel;
            }
        }

        /* direct addressing by client
         * for subsampled planes, scale will change
         */
        for (y = 0; y < addr.dim_y; y+=addr.step_y) {
            for (x = 0; x < addr.dim_x; x+=addr.step_x) {
                vx_uint8 *tmp = (vx_uint8 *)base_ptr;
                i = ((addr.stride_y*y*addr.scale_y) /
                      VX_SCALE_UNITY) +
                    ((addr.stride_x*x*addr.scale_x) /
                      VX_SCALE_UNITY);
                tmp[i] = pixel;
            }
        }

        /* more efficient direct addressing by client.
         * for subsampled planes, scale will change.
         */
        for (y = 0; y < addr.dim_y; y+=addr.step_y) {
            j = (addr.stride_y*y*addr.scale_y)/VX_SCALE_UNITY;
            for (x = 0; x < addr.dim_x; x+=addr.step_x) {
                vx_uint8 *tmp = (vx_uint8 *)base_ptr;
                i = j + (addr.stride_x*x*addr.scale_x) /
                    VX_SCALE_UNITY;
                tmp[i] = pixel;
            }
        }

        /* this commits the data back to the image. If rect were 0 or empty, it
         * would just decrement the reference (used when reading an image only)
         */
        status = vxCommitImagePatch(image, &rect, plane, &addr, base_ptr);
    }
    vxReleaseImage(&image);
    //! [imagepatch]
    return status;
}

