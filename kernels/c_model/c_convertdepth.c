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

#include <VX/vx_types.h>
#include <VX/vx_lib_extras.h>
#include <stdio.h>


// nodeless version of the ConvertDepth kernel
vx_status vxConvertDepth(vx_image input, vx_image output, vx_scalar spol, vx_scalar sshf)
{
    vx_uint32 y, x;
    void *dst_base = NULL;
    void *src_base = NULL;
    vx_imagepatch_addressing_t dst_addr, src_addr;
    vx_rectangle_t rect;
    vx_enum format[2];
    vx_enum policy = 0;
    vx_int32 shift = 0;

    vx_status status = VX_SUCCESS;
    status |= vxAccessScalarValue(spol, &policy);
    status |= vxAccessScalarValue(sshf, &shift);
    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format[0], sizeof(format[0]));
    status |= vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format[1], sizeof(format[1]));
    status |= vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
    for (y = 0; y < src_addr.dim_y; y++)
    {
        for (x = 0; x < src_addr.dim_x; x++)
        {
            if ((format[0] == VX_DF_IMAGE_U8) && (format[1] == VX_DF_IMAGE_U16))
            {
                vx_uint8 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                *dst = ((vx_uint16)(*src)) << shift;
            }
            else if ((format[0] == VX_DF_IMAGE_U8) && (format[1] == VX_DF_IMAGE_S16))
            {
                vx_uint8 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_int16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                *dst = ((vx_int16)(*src)) << shift;
            }
            else if ((format[0] == VX_DF_IMAGE_U8) && (format[1] == VX_DF_IMAGE_U32))
            {
                vx_uint8 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint32 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                *dst = ((vx_uint32)(*src)) << shift;
            }
            else if ((format[0] == VX_DF_IMAGE_U16) && (format[1] == VX_DF_IMAGE_U32))
            {
                vx_uint16 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint32 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                *dst = ((vx_uint32)(*src)) << shift;
            }
            else if ((format[0] == VX_DF_IMAGE_S16) && (format[1] == VX_DF_IMAGE_S32))
            {
                vx_int16 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_int32 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                *dst = ((vx_int32)(*src)) << shift;
            }
            else if ((format[0] == VX_DF_IMAGE_U16) && (format[1] == VX_DF_IMAGE_U8))
            {
                vx_uint16 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (policy == VX_CONVERT_POLICY_WRAP)
                {
                    *dst = (vx_uint8)((*src) >> shift);
                }
                else if (policy == VX_CONVERT_POLICY_SATURATE)
                {
                    vx_uint16 value = (*src) >> shift;
                    value = (value > UINT8_MAX ? UINT8_MAX : value);
                    *dst = (vx_uint8)value;
                }
            }
            else if ((format[0] == VX_DF_IMAGE_S16) && (format[1] == VX_DF_IMAGE_U8))
            {
                vx_int16 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (policy == VX_CONVERT_POLICY_WRAP)
                {
                    *dst = (vx_uint8)((*src) >> shift);
                }
                else if (policy == VX_CONVERT_POLICY_SATURATE)
                {
                    vx_int16 value = (*src) >> shift;
                    value = (value < 0 ? 0 : value);
                    value = (value > UINT8_MAX ? UINT8_MAX : value);
                    *dst = (vx_uint8)value;
                }
            }
            else if ((format[0] == VX_DF_IMAGE_U32) && (format[1] == VX_DF_IMAGE_U8))
            {
                vx_uint32 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (policy == VX_CONVERT_POLICY_WRAP)
                {
                    *dst = (vx_uint8)((*src) >> shift);
                }
                else if (policy == VX_CONVERT_POLICY_SATURATE)
                {
                    vx_uint32 value = (*src) >> shift;
                    value = (value > UINT8_MAX ? UINT8_MAX : value);
                    *dst = (vx_uint8)value;
                }
            }
            else if ((format[0] == VX_DF_IMAGE_U32) && (format[1] == VX_DF_IMAGE_U16))
            {
                vx_uint32 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (policy == VX_CONVERT_POLICY_WRAP)
                {
                    *dst = (vx_uint16)((*src) >> shift);
                }
                else if (policy == VX_CONVERT_POLICY_SATURATE)
                {
                    vx_uint32 value = (*src) >> shift;
                    value = (value > UINT16_MAX ? UINT16_MAX : value);
                    *dst = (vx_uint16)value;
                }
            }
            else if ((format[0] == VX_DF_IMAGE_S32) && (format[1] == VX_DF_IMAGE_S16))
            {
                vx_int32 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_int16 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                if (policy == VX_CONVERT_POLICY_WRAP)
                {
                    *dst = (vx_int16)((*src) >> shift);
                }
                else if (policy == VX_CONVERT_POLICY_SATURATE)
                {
                    vx_int32 value = (*src) >> shift;
                    value = (value < INT16_MIN ? INT16_MIN : value);
                    value = (value > INT16_MAX ? INT16_MAX : value);
                    *dst = (vx_int16)value;
                }
            }
            else if ((format[0] == VX_DF_IMAGE_F32) && (format[1] == VX_DF_IMAGE_U8))
            {
                vx_float32 *src = vxFormatImagePatchAddress2d(src_base, x, y, &src_addr);
                vx_uint8 *dst = vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                vx_float32 pf = floorf(log10f(*src));
                vx_int32 p32 = (vx_int32)pf;
                vx_uint8 p8 = (vx_uint8)(p32 > 255 ? 255 : (p32 < 0 ? 0 : p32));
                if (pf > 0.0f)
                {
//                    printf("Float power of %lf\n", pf); //removed this print. You get too much of those prints in harris corner
                }
                *dst = (vx_uint8)(p8 << shift);
            }
        }
    }
    status |= vxCommitImagePatch(input, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);

    return status;
}

