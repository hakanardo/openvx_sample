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
#include <vx_debug.h>

// nodeless version of the MeanStdDev kernel
vx_status vxMeanStdDev(vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_float32 fmean = 0.0f, sum = 0.0f, sum_diff_sqrs = 0.0f, fstddev = 0.0f;
    vx_df_image format = 0;
    vx_rectangle_t rect ;
    vx_imagepatch_addressing_t addrs;
    void *base_ptr = NULL;
    vx_uint32 x, y;
    vx_status status  = VX_SUCCESS;

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    status = vxGetValidRegionImage(input, &rect);
    //VX_PRINT(VX_ZONE_INFO, "Rectangle = {%u,%u x %u,%u}\n",rect.start_x, rect.start_y, rect.end_x, rect.end_y);
    status |= vxAccessImagePatch(input, &rect, 0, &addrs, &base_ptr, VX_READ_ONLY);
    for (y = 0; y < addrs.dim_y; y++)
    {
        for (x = 0; x < addrs.dim_x; x++)
        {
            if (format == VX_DF_IMAGE_U8)
            {
                vx_uint8 *pixel = vxFormatImagePatchAddress2d(base_ptr, x, y, &addrs);
                sum += (vx_float32)(*pixel);
            }
            else if (format == VX_DF_IMAGE_U16)
            {
                vx_uint16 *pixel = vxFormatImagePatchAddress2d(base_ptr, x, y, &addrs);
                sum += (vx_float32)(*pixel);
            }
        }
    }
    fmean = sum / (addrs.dim_x*addrs.dim_y);
    for (y = 0; y < addrs.dim_y; y++)
    {
        for (x = 0; x < addrs.dim_x; x++)
        {
            if (format == VX_DF_IMAGE_U8)
            {
                vx_uint8 *pixel = vxFormatImagePatchAddress2d(base_ptr, x, y, &addrs);
                sum_diff_sqrs += (vx_float32)pow((vx_float32)(*pixel) - fmean, 2);
            }
            else if (format == VX_DF_IMAGE_U16)
            {
                vx_uint16 *pixel = vxFormatImagePatchAddress2d(base_ptr, x, y, &addrs);
                sum_diff_sqrs += (vx_float32)pow((vx_float32)(*pixel) - fmean, 2);
            }
        }
    }
    fstddev = (vx_float32)sqrt(sum_diff_sqrs / (addrs.dim_x*addrs.dim_y));
    status |= vxCommitScalarValue(mean, &fmean);
    status |= vxCommitScalarValue(stddev, &fstddev);
    status |= vxCommitImagePatch(input, &rect, 0, &addrs, base_ptr);

    return status;
}

// nodeless version of the MinMaxLoc kernel
static void analyzeMinMaxValue(vx_uint32 x, vx_uint32 y, vx_int64 v,
                               vx_int64 *pMinVal, vx_int64 *pMaxVal,
                               vx_uint32 *pMinCount, vx_uint32 *pMaxCount,
                               vx_array minLoc, vx_array maxLoc)
{
    vx_coordinates2d_t loc;
    vx_size minLocCapacity = 0, maxLocCapacity = 0;

    loc.x = x;
    loc.y = y;

    vxQueryArray(minLoc, VX_ARRAY_ATTRIBUTE_CAPACITY, &minLocCapacity, sizeof(minLocCapacity));
    vxQueryArray(maxLoc, VX_ARRAY_ATTRIBUTE_CAPACITY, &maxLocCapacity, sizeof(maxLocCapacity));

    if (v > *pMaxVal)
    {
        *pMaxVal = v;
        *pMaxCount = 1;
        if (maxLoc)
        {
            vxTruncateArray(maxLoc, 0);
            vxAddArrayItems(maxLoc, 1, &loc, 0);
        }
    }
    else if (v == *pMaxVal)
    {
        if (maxLoc && *pMaxCount < maxLocCapacity)
        {
            vxAddArrayItems(maxLoc, 1, &loc, 0);
        }
        ++(*pMaxCount);
    }

    if (v < *pMinVal)
    {
        *pMinVal = v;
        *pMinCount = 1;
        if (minLoc)
        {
            vxTruncateArray(minLoc, 0);
            vxAddArrayItems(minLoc, 1, &loc, 0);
        }
    }
    else if (v == *pMinVal)
    {
        if (minLoc && *pMinCount < minLocCapacity)
        {
            vxAddArrayItems(minLoc, 1, &loc, 0);
        }
        ++(*pMinCount);
    }
}

vx_status vxMinMaxLoc(vx_image input, vx_scalar minVal, vx_scalar maxVal, vx_array minLoc, vx_array maxLoc, vx_scalar minCount, vx_scalar maxCount)
{
    vx_uint32 y, x;
    void *src_base = NULL;
    vx_imagepatch_addressing_t src_addr;
    vx_rectangle_t rect;
    vx_df_image format;
    vx_int64 iMinVal = INT64_MAX;
    vx_int64 iMaxVal = INT64_MIN;
    vx_uint32 iMinCount = 0;
    vx_uint32 iMaxCount = 0;
    vx_status status = VX_SUCCESS;

    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    status |= vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &src_addr, (void **)&src_base, VX_READ_ONLY);
    for (y = 0; y < src_addr.dim_y; y++)
    {
        for (x = 0; x < src_addr.dim_x; x++)
        {
            void *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
            if (format == VX_DF_IMAGE_U8)
            {
                vx_uint8 v = *(vx_uint8 *)src;
                analyzeMinMaxValue(x, y, v, &iMinVal, &iMaxVal, &iMinCount, &iMaxCount, minLoc, maxLoc);
            }
            else if (format == VX_DF_IMAGE_U16)
            {
                vx_uint16 v = *(vx_uint16 *)src;
                analyzeMinMaxValue(x, y, v, &iMinVal, &iMaxVal, &iMinCount, &iMaxCount, minLoc, maxLoc);
            }
            else if (format == VX_DF_IMAGE_U32)
            {
                vx_uint32 v = *(vx_uint32 *)src;
                analyzeMinMaxValue(x, y, v, &iMinVal, &iMaxVal, &iMinCount, &iMaxCount, minLoc, maxLoc);
            }
            else if (format == VX_DF_IMAGE_S16)
            {
                vx_int16 v = *(vx_int16 *)src;
                analyzeMinMaxValue(x, y, v, &iMinVal, &iMaxVal, &iMinCount, &iMaxCount, minLoc, maxLoc);
            }
            else if (format == VX_DF_IMAGE_S32)
            {
                vx_int32 v = *(vx_int32 *)src;
                analyzeMinMaxValue(x, y, v, &iMinVal, &iMaxVal, &iMinCount, &iMaxCount, minLoc, maxLoc);
            }
        }
    }
    VX_PRINT(VX_ZONE_INFO, "Min = %ld Max = %ld\n", iMinVal, iMaxVal);
    status |= vxCommitImagePatch(input, NULL, 0, &src_addr, src_base);

    switch (format)
    {
        case VX_DF_IMAGE_U8:
            {
                vx_uint8 min = (vx_uint8)iMinVal, max = (vx_uint8)iMaxVal;
                if (minVal) vxCommitScalarValue(minVal, &min);
                if (maxVal) vxCommitScalarValue(maxVal, &max);
            }
            break;
        case VX_DF_IMAGE_U16:
            {
                vx_uint16 min = (vx_uint16)iMinVal, max = (vx_uint16)iMaxVal;
                if (minVal) vxCommitScalarValue(minVal, &min);
                if (maxVal) vxCommitScalarValue(maxVal, &max);
            }
            break;
        case VX_DF_IMAGE_U32:
            {
                vx_uint32 min = (vx_uint32)iMinVal, max = (vx_uint32)iMaxVal;
                if (minVal) vxCommitScalarValue(minVal, &min);
                if (maxVal) vxCommitScalarValue(maxVal, &max);
            }
            break;
        case VX_DF_IMAGE_S16:
            {
                vx_int16 min = (vx_int16)iMinVal, max = (vx_int16)iMaxVal;
                if (minVal) vxCommitScalarValue(minVal, &min);
                if (maxVal) vxCommitScalarValue(maxVal, &max);
            }
            break;
        case VX_DF_IMAGE_S32:
            {
                vx_int32 min = (vx_int32)iMinVal, max = (vx_int32)iMaxVal;
                if (minVal) vxCommitScalarValue(minVal, &min);
                if (maxVal) vxCommitScalarValue(maxVal, &max);
            }
            break;
    }

    if (minCount) vxCommitScalarValue(minCount, &iMinCount);
    if (maxCount) vxCommitScalarValue(maxCount, &iMaxCount);

    return status;
}

