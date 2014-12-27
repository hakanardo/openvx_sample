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
#include <stdio.h>

// nodeless version of the Histogram kernel
vx_status vxHistogram(vx_image src, vx_distribution dist)
{
    vx_rectangle_t src_rect;
    vx_imagepatch_addressing_t src_addr;
    void* src_base = NULL;
    void* dist_ptr = NULL;
    vx_df_image format = 0;
    vx_uint32 x = 0;
    vx_uint32 y = 0;
    vx_size offset = 0;
    vx_size range = 0;
    vx_size numBins = 0;
    vx_uint32 window_size = 0;
    vx_status status = VX_SUCCESS;

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_BINS, &numBins, sizeof(numBins));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_RANGE, &range, sizeof(range));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_OFFSET, &offset, sizeof(offset));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_WINDOW, &window_size, sizeof(window_size));
    status = vxGetValidRegionImage(src, &src_rect);
    status |= vxAccessImagePatch(src, &src_rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessDistribution(dist, &dist_ptr, VX_WRITE_ONLY);
    //printf("distribution:%p bins:%u off:%u ws:%u range:%u\n", dist_ptr, numBins, offset, window_size, range);
    if (status == VX_SUCCESS)
    {
        vx_int32 *dist_tmp = dist_ptr;

        /* clear the distribution */
        for (x = 0; x < numBins; x++)
        {
            dist_tmp[x] = 0;
        }

        for (y = 0; y < src_addr.dim_y; y++)
        {
            for (x = 0; x < src_addr.dim_x; x++)
            {
                if (format == VX_DF_IMAGE_U8)
                {
                    vx_uint8 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                    vx_uint8 pixel = *src_ptr;
                    if ((offset <= (vx_size)pixel) && ((vx_size)pixel < (offset+range)))
                    {
                        vx_size index = (pixel - (vx_uint16)offset) / window_size;
                        dist_tmp[index]++;
                    }
                }
                else if (format == VX_DF_IMAGE_U16)
                {
                    vx_uint16 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                    vx_uint16 pixel = *src_ptr;
                    if ((offset <= (vx_size)pixel) && ((vx_size)pixel < (offset+range)))
                    {
                        vx_size index = (pixel - (vx_uint16)offset) / window_size;
                        dist_tmp[index]++;
                    }
                }
            }
        }
    }
    status |= vxCommitDistribution(dist, dist_ptr);
    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);

    return status;
}

// nodeless version of the EqualizeHist kernel
vx_status vxEqualizeHist(vx_image src, vx_image dst)
{
    vx_uint32 y, x;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_rectangle_t rect;
    vx_status status = VX_SUCCESS;

    status = vxGetValidRegionImage(src, &rect);
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
    if (status == VX_SUCCESS)
    {
        /* for 16-bit support (U16 or S16), the code can be duplicated with NUM_BINS = 65536 and PIXEL = vx_uint16. */
        #define NUM_BINS 256

        /* allocate a fixed-size temp array to store the image histogram & cumulative distribution */
        vx_uint32 hist[NUM_BINS] = {0};
        vx_uint32 cdf[NUM_BINS] = {0};
        vx_uint32 sum = 0, div;
        vx_uint8 minv = 0xFF;

        /* calculate the distribution (histogram) */
        for (y = 0; y < src_addr.dim_y; y++)
        {
            for (x = 0; x < src_addr.dim_x; x++)
            {
                vx_uint8 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 pixel = (*src_ptr);
                hist[pixel]++;
                if (minv > pixel)
                    minv = pixel;
            }
        }
        /* calculate the cumulative distribution (summed histogram) */
        for (x = 0; x < NUM_BINS; x++)
        {
            sum += hist[x];
            cdf[x] = sum;
        }
        div = (src_addr.dim_x * src_addr.dim_y) - cdf[minv];
        if( div > 0 )
        {
            /* recompute the histogram to be a LUT for replacing pixel values */
            for (x = 0; x < NUM_BINS; x++)
            {
                uint32_t cdfx = cdf[x] - cdf[minv];
                vx_float32 p = (vx_float32)cdfx/(vx_float32)div;
                hist[x] = (uint8_t)(p * 255.0f + 0.5f);
            }
        }
        else
        {
            for (x = 0; x < NUM_BINS; x++)
            {
                hist[x] = x;
            }
        }

        /* map the src pixel values to the equalized pixel values */
        for (y = 0; y < src_addr.dim_y; y++)
        {
            for (x = 0; x < src_addr.dim_x; x++)
            {
                vx_uint8 *src_ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 *dst_ptr = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                *dst_ptr = hist[(*src_ptr)];
            }
        }
    }

    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}

