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

#include <VX/vx_khr_tiling.h>

/*! \file
 * \example vx_tiling_alpha.c
 */

/*! \brief A 1x1 to 1x1 tile size.
 * \details This tiling kernel doesn't have an inner set of block loops as the
 * block size is 1x1.
 * This kernel uses this tiling definition.
 * \code
 * vx_block_size_t outSize = {1,1};
 * vx_neighborhood_size_t inNbhd = {0,0,0,0};
 * \endcode
 * \ingroup group_tiling
 */
//! [alpha_tiling_function]
void alpha_image_tiling(void * VX_RESTRICT parameters[VX_RESTRICT],
                        void * VX_RESTRICT tile_memory,
                        vx_size tile_memory_size)
{
    vx_uint32 i,j;
    vx_tile_t *in = (vx_tile_t *)parameters[0];
    vx_float32 *alpha = (vx_float32 *)parameters[1];
    vx_tile_t *out = (vx_tile_t *)parameters[2];

    for (j = 0u; j < vxTileHeight(in, 0); j+=vxTileBlockHeight(in))
    {
        for (i = 0u; i < vxTileWidth(in, 0); i+=vxTileBlockWidth(in))
        {
            vx_uint8 pixel = vxImagePixel(vx_uint8, in, 0, i, j, 0, 0);
            vx_uint8 coeff = (vx_uint8)(255.0f * (*alpha));
            vx_uint16 value = (coeff * pixel) >> 8;
            if (value > 255)
                value = 255;
            vxImagePixel(vx_uint8, out, 0, i, j, 0, 0) = (vx_uint8)value;
        }
    }
}
//! [alpha_tiling_function]
