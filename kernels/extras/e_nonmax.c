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

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <math.h>
#include <extras_k.h>

// nodeless version of the EuclideanNonMaxSuppression kernel
vx_status vxEuclideanNonMaxSuppression(vx_image src, vx_scalar thr, vx_scalar rad, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    void *src_base = NULL, *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_float32 radius = 0.0f;
    vx_int32 r = 0;
    vx_float32 thresh = 0;
    vx_int32 threshold = 0;
    vx_rectangle_t rect;
    vx_df_image format = VX_DF_IMAGE_VIRT;

    status = vxGetValidRegionImage(src, &rect);
    status |= vxAccessScalarValue(rad, &radius);
    status |= vxAccessScalarValue(thr, &thresh);
    status |= vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
    r = (vx_uint32)radius;
    r = (r <=0 ? 1 : r);
    threshold = (vx_int32)thresh;
    if (status == VX_SUCCESS)
    {
        vx_uint32 y, x;
        vx_int32 i, j;
        vx_float32 d = 0;
        for (y = 0; y < src_addr.dim_y; y++)
        {
            for (x = 0; x < src_addr.dim_x; x++)
            {
                if (format == VX_DF_IMAGE_S32)
                {
                    vx_int32 max = 0;
                    vx_int32 *ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                    vx_int32 *out = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                    if ((*ptr) < threshold)
                    {
                        *out = 0.f;
                        continue;
                    }
                    //printf("src(%d,%d) = %d > %d (r2=%u)\n",x,y,*ptr,threshold,r2);
                    for (j = -r; j <= r; j++)
                    {
                        if ((y+j < 0) || (y+j >= src_addr.dim_y))
                        {
                            continue;
                        }
                        for (i = -r; i <= r; i++)
                        {
                            if ((x+i < 0) || (x+i >= src_addr.dim_x))
                            {
                                continue;
                            }
                            else
                            {
                                vx_int32 dx = (x - (x+i));
                                vx_int32 dy = (y - (y+j));
                                d = sqrtf((vx_float32)((dx*dx) + (dy*dy)));
                                //printf("{%d,%d} is %lf from {%d,%d} radius=%lf\n",x+i,y+j,d,x,y,radius);
                                if (d < radius)
                                {
                                    vx_int32 *non = vxFormatImagePatchAddress2d(src_base, x+i, y+j, &src_addr);
                                    //printf("src(%d,%d) = %d ? %d\n",x+i,y+j,(*non),max);
                                    if (max < (*non))
                                        max = (*non);
                                }
                            }
                        }
                    }
                    //printf("src(%d,%d) = %d ? %d\n",x+i,y+j,(*ptr),max);
                    if ((*ptr) == max)
                        *out = *ptr;
                    else
                        *out = 0;
                }
                else if (format == VX_DF_IMAGE_F32)
                {
                    vx_float32 max = 0.0f;
                    vx_float32 *ptr = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                    vx_float32 *out = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                    if ((*ptr) < thresh)
                    {
                        *out = 0.f;
                        continue;
                    }
                    //printf("src(%d,%d) = %d > %d (r2=%u)\n",x,y,*ptr,threshold,r2);
                    for (j = -r; j <= r; j++)
                    {
                        if ((y+j < 0) || (y+j >= src_addr.dim_y))
                        {
                            continue;
                        }
                        for (i = -r; i <= r; i++)
                        {
                            if ((x+i < 0) || (x+i >= src_addr.dim_x))
                            {
                                continue;
                            }
                            else
                            {
                                vx_float32 dx = (vx_float32)(x - (x+i));
                                vx_float32 dy = (vx_float32)(y - (y+j));
                                d = sqrtf((dx*dx) + (dy*dy));
                                //printf("{%d,%d} is %lf from {%d,%d} radius=%lf\n",x+i,y+j,d,x,y,radius);
                                if (d < radius)
                                {
                                    vx_float32 *non = vxFormatImagePatchAddress2d(src_base, x+i, y+j, &src_addr);
                                    //printf("src(%d,%d) = %d ? %d\n",x+i,y+j,(*non),max);
                                    if (max < (*non))
                                        max = (*non);
                                }
                            }
                        }
                    }
                    //printf("src(%d,%d) = %d ? %d\n",x+i,y+j,(*ptr),max);
                    if ((*ptr) == max)
                        *out = *ptr;
                    else
                        *out = 0;
                }
            }
        }
    }
    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}


static const int neighbor_indexes[][2] = {
    {3, 5},
    {6, 2},
    {7, 1},
    {8, 0},
    {5, 3},
    {2, 6},
    {1, 7},
    {0, 8},
    {3, 5},
};

// nodeless version of the NonMaxSuppression kernel
vx_status vxNonMaxSuppression(vx_image i_mag, vx_image i_ang, vx_image i_edge, vx_border_mode_t *borders)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 y = 0, x = 0;
    void *mag_base = NULL;
    void *ang_base = NULL;
    void *edge_base = NULL;
    vx_imagepatch_addressing_t mag_addr, ang_addr, edge_addr;
    vx_rectangle_t rect;
    vx_df_image format = 0;
    vx_uint32 low_x = 0, high_x;
    vx_uint32 low_y = 0, high_y;

    status  = VX_SUCCESS; // assume success until an error occurs.
    status |= vxQueryImage(i_mag, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    status |= vxGetValidRegionImage(i_mag, &rect);
    status |= vxAccessImagePatch(i_mag, &rect, 0, &mag_addr, &mag_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(i_ang, &rect, 0, &ang_addr, &ang_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(i_edge, &rect, 0, &edge_addr, &edge_base, VX_WRITE_ONLY);

    high_x = edge_addr.dim_x;
    high_y = edge_addr.dim_y;

    if (borders->mode == VX_BORDER_MODE_UNDEFINED)
    {
        ++low_x; --high_x;
        ++low_y; --high_y;
        vxAlterRectangle(&rect, 1, 1, -1, -1);
    }

    for (y = low_y; y < high_y; y++)
    {
        for (x = low_x; x < high_x; x++)
        {
            vx_uint8 *ang = vxFormatImagePatchAddress2d(ang_base, x, y, &ang_addr);
            vx_uint8 angle = *ang;
            if (format == VX_DF_IMAGE_U8)
            {
                vx_uint8 mag[9];
                vx_uint8 *edge = vxFormatImagePatchAddress2d(edge_base, x, y, &edge_addr);
                const int *ni = neighbor_indexes[(angle + 16) / 32];
                vxReadRectangle(mag_base, &mag_addr, borders, format, x, y, 1, 1, mag);
                *edge = mag[4] > mag[ni[0]] && mag[4] > mag[ni[1]] ? mag[4] : 0;
            }
            else if (format == VX_DF_IMAGE_S16)
            {
                vx_int16 mag[9];
                vx_int16 *edge = vxFormatImagePatchAddress2d(edge_base, x, y, &edge_addr);
                const int *ni = neighbor_indexes[(angle + 16) / 32];
                vxReadRectangle(mag_base, &mag_addr, borders, format, x, y, 1, 1, mag);
                *edge = mag[4] > mag[ni[0]] && mag[4] > mag[ni[1]] ? mag[4] : 0;
            }
            else if (format == VX_DF_IMAGE_U16)
            {
                vx_uint16 mag[9];
                vx_uint16 *edge = vxFormatImagePatchAddress2d(edge_base, x, y, &edge_addr);
                const int *ni = neighbor_indexes[(angle + 16) / 32];
                vxReadRectangle(mag_base, &mag_addr, borders, format, x, y, 1, 1, mag);
                *edge = mag[4] > mag[ni[0]] && mag[4] > mag[ni[1]] ? mag[4] : 0;
            }
        }
    }

    status |= vxCommitImagePatch(i_mag, NULL, 0, &mag_addr, mag_base);
    status |= vxCommitImagePatch(i_ang, NULL, 0, &ang_addr, ang_base);
    status |= vxCommitImagePatch(i_edge, &rect, 0, &edge_addr, edge_base);

    return status;
}
