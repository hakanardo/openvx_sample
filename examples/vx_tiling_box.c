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

/*! \file
 * \example vx_tiling_box.c
 */

/*! \brief A 3x3 to 1x1 box filter
 * This kernel uses this tiling definition.
 * \code
 * vx_block_size_t outSize = {1,1};
 * vx_neighborhood_size_t inNbhd = {-1,1,-1,1};
 * \endcode
 * \ingroup group_tiling
 */
//! [box_tiling_function]
void box_image_tiling(void * VX_RESTRICT parameters[VX_RESTRICT],
                      void * VX_RESTRICT tile_memory,
                      vx_size tile_memory_size)
{
    vx_uint32 x, y;
    vx_tile_t *in = (vx_tile_t *)parameters[0];
    vx_tile_t *out = (vx_tile_t *)parameters[1];

    for (y = 0; y < vxTileHeight(out, 0); y+=vxTileBlockHeight(out))
    {
        for (x = 0; x < vxTileWidth(out, 0); x+=vxTileBlockWidth(out))
        {
            vx_int32 j, i;
            vx_uint32 sum = 0;
            vx_uint32 count = 0;
            /* these loops can handle 3x3, 5x5, etc. since block size would be 1x1 */
            for (j = vxNeighborhoodTop(in); j < vxNeighborhoodBottom(in); j++)
            {
                for (i = vxNeighborhoodLeft(in); i < vxNeighborhoodRight(in); i++)
                {
                    sum += vxImagePixel(vx_uint8, in, 0, x, y, i, j);
                    count++;
                }
            }
            sum /= count;
            if (sum > 255)
                sum = 255;
            vxImagePixel(vx_uint8, out, 0, x, y, 0, 0) = (vx_uint8)sum;
        }
    }
}
//! [box_tiling_function]

