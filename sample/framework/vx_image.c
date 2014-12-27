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

#include <vx_internal.h>

#if defined(EXPERIMENTAL_USE_OPENCL)

typedef struct _vx_cl_image_format_t {
    cl_mem_object_type cl_type;
    cl_image_format  cl_format;
    vx_df_image        vx_format;
    vx_size          stride_x;
} vx_cl_image_format;

vx_cl_image_format image_convert_format[] = {
#if !defined(CL_USE_IMAGES)
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_U8, sizeof(vx_uint8)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_U16, sizeof(vx_uint16)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_S16, sizeof(vx_int16)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_S32, sizeof(vx_int32)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_U32, sizeof(vx_uint32)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_RGB, sizeof(vx_uint32)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_RGBX, sizeof(vx_uint32)},
#else
    /*************************************************************
    [Bug report]OpenCL specification says CL_LUMINANCE not support 
    CL_UNSIGNED_INT8, CL_UNSIGNED_INT16, 
    CL_SIGNED_INT16, CL_SIGNED_INT32 at all !!!
    
    CL_LUMINANCE.  
    This format can only be used if channel data type = 
    CL_UNORM_INT8, CL_UNORM_INT16, CL_SNORM_INT8, CL_SNORM_INT16, 
    CL_HALF_FLOAT or CL_FLOAT. 
    Please read OpenCL Specification carefully!!
    *************************************************************/
    {CL_MEM_OBJECT_IMAGE2D, {CL_LUMINANCE, CL_UNSIGNED_INT8},        VX_DF_IMAGE_U8, sizeof(vx_uint8)},
    {CL_MEM_OBJECT_IMAGE2D, {CL_LUMINANCE, CL_UNSIGNED_INT16},       VX_DF_IMAGE_U16, sizeof(vx_uint16)},
    {CL_MEM_OBJECT_IMAGE2D, {CL_LUMINANCE, CL_SIGNED_INT16},       VX_DF_IMAGE_S16, sizeof(vx_int16)},
    {CL_MEM_OBJECT_IMAGE2D, {CL_LUMINANCE, CL_SIGNED_INT32},      VX_DF_IMAGE_S32, sizeof(vx_int32)},
    {CL_MEM_OBJECT_IMAGE2D, {CL_LUMINANCE, CL_UNSIGNED_INT32},    VX_DF_IMAGE_U32, sizeof(vx_uint32)},
    {CL_MEM_OBJECT_IMAGE2D, {CL_RGBA,      CL_UNORM_INT8},        VX_DF_IMAGE_RGB, sizeof(vx_uint8)*4},
    {CL_MEM_OBJECT_IMAGE2D, {CL_RGBA,      CL_UNORM_INT8},        VX_DF_IMAGE_RGBX, sizeof(vx_uint8)*4},
#endif
#if defined(CL_USE_IMAGES) && defined(CL_VERSION_1_2) && defined(__APPLE__)
    {CL_MEM_OBJECT_IMAGE2D, {CL_YCbYCr_APPLE, CL_UNORM_INT8},     VX_DF_IMAGE_UYVY, sizeof(vx_uint16)},
    {CL_MEM_OBJECT_IMAGE2D, {CL_CbYCrY_APPLE, CL_UNORM_INT8},     VX_DF_IMAGE_YUYV, sizeof(vx_uint16)},
#else
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_UYVY, sizeof(vx_uint16)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_YUYV, sizeof(vx_uint16)},
#endif
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_IYUV, sizeof(vx_uint8)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_NV12, sizeof(vx_uint8)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_NV21, sizeof(vx_uint8)},
    {CL_MEM_OBJECT_BUFFER, {0, 0}, VX_DF_IMAGE_YUV4, sizeof(vx_uint8)},
};
#endif

static vx_uint32 vxComputePatchOffset(vx_uint32 x, vx_uint32 y, vx_imagepatch_addressing_t *addr)
{
    return ((addr->stride_y * ((addr->scale_y * y)/VX_SCALE_UNITY)) +
             (addr->stride_x * ((addr->scale_x * x)/VX_SCALE_UNITY)));
}

static vx_uint32 vxComputePlaneOffset(vx_image image, vx_uint32 x, vx_uint32 y, vx_uint32 p)
{
    return  (((y * image->memory.strides[p][VX_DIM_Y]) / image->scale[p][VX_DIM_Y]) +
              ((x * image->memory.strides[p][VX_DIM_X]) / image->scale[p][VX_DIM_X]));
}

static vx_uint32 vxComputePatchRangeSize(vx_uint32 range, vx_imagepatch_addressing_t *addr)
{
    return (range * addr->stride_x * addr->scale_x) / VX_SCALE_UNITY;
}

static vx_uint32 vxComputePlaneRangeSize(vx_image image, vx_uint32 range, vx_uint32 p)
{
    return (range * image->memory.strides[p][VX_DIM_X]) / image->scale[p][VX_DIM_X];
}

vx_bool vxIsValidImage(vx_image image)
{
    if ((vxIsValidSpecificReference(&image->base, VX_TYPE_IMAGE) == vx_true_e) &&
        (vxIsSupportedFourcc(image->format) == vx_true_e))
    {
        return vx_true_e;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Image!\n");
        return vx_false_e;
    }
}

vx_bool vxIsSupportedFourcc(vx_df_image code)
{
    switch (code)
    {
        case VX_DF_IMAGE_RGB:
        case VX_DF_IMAGE_RGBX:
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_IYUV:
        case VX_DF_IMAGE_YUV4:
        case VX_DF_IMAGE_U8:
        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
        case VX_DF_IMAGE_VIRT:
            return vx_true_e;
        default:
            VX_PRINT(VX_ZONE_ERROR, "Format 0x%08x is not supported\n", code);
            if (code != 0) DEBUG_BREAK();
            return vx_false_e;
    }
}

static vx_size vxSizeOfChannel(vx_df_image color)
{
    vx_size size = 0ul;
    if (vxIsSupportedFourcc(color))
    {
        switch (color)
        {
            case VX_DF_IMAGE_S16:
            case VX_DF_IMAGE_U16:
                size = sizeof(vx_uint16);
                break;
            case VX_DF_IMAGE_U32:
            case VX_DF_IMAGE_S32:
            case VX_DF_IMAGE_F32:
                size = sizeof(vx_uint32);
                break;
            default:
                size = 1ul;
                break;
        }
    }
    return size;
}

void vxInitPlane(vx_image image,
                 vx_uint32 index,
                 vx_uint32 soc,
                 vx_uint32 channels,
                 vx_uint32 width,
                 vx_uint32 height)
{
    if (image)
    {
        image->memory.strides[index][VX_DIM_C] = soc;
        image->memory.dims[index][VX_DIM_C] = channels;
        image->memory.dims[index][VX_DIM_X] = width;
        image->memory.dims[index][VX_DIM_Y] = height;
        image->memory.ndims = VX_DIM_MAX;
        image->scale[index][VX_DIM_C] = 1;
        image->scale[index][VX_DIM_X] = 1;
        image->scale[index][VX_DIM_Y] = 1;
        image->bounds[index][VX_DIM_C][VX_BOUND_START] = 0;
        image->bounds[index][VX_DIM_C][VX_BOUND_END] = channels;
        image->bounds[index][VX_DIM_X][VX_BOUND_START] = 0;
        image->bounds[index][VX_DIM_X][VX_BOUND_END] = width;
        image->bounds[index][VX_DIM_Y][VX_BOUND_START] = 0;
        image->bounds[index][VX_DIM_Y][VX_BOUND_END] = height;
    }
}

void vxInitImage(vx_image image, vx_uint32 width, vx_uint32 height, vx_df_image color)
{
    vx_uint32 soc = (vx_uint32)vxSizeOfChannel(color);
    image->width = width;
    image->height = height;
    image->format = color;
    image->range = VX_CHANNEL_RANGE_FULL;
    image->import_type = VX_IMPORT_TYPE_NONE;
    /* when an image is allocated, it's not valid until it's been written to.
     * this inverted rectangle is needed for the initial write case.
     */
    image->region.start_x = width;
    image->region.start_y = height;
    image->region.end_x = 0;
    image->region.end_y = 0;
    switch (image->format)
    {
        case VX_DF_IMAGE_U8:
        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
            image->space = VX_COLOR_SPACE_NONE;
            break;
        default:
            image->space = VX_COLOR_SPACE_DEFAULT;
            break;
    }

    switch (image->format)
    {
        case VX_DF_IMAGE_VIRT:
            break;
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
            image->planes = 2;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            vxInitPlane(image, 1, soc, 2, image->width/2, image->height/2);
            image->scale[1][VX_DIM_X] = 2;
            image->scale[1][VX_DIM_Y] = 2;
            image->bounds[1][VX_DIM_X][VX_BOUND_END] *= image->scale[1][VX_DIM_X];
            image->bounds[1][VX_DIM_Y][VX_BOUND_END] *= image->scale[1][VX_DIM_Y];
            break;
        case VX_DF_IMAGE_RGB:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 3, image->width, image->height);
            break;
        case VX_DF_IMAGE_RGBX:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 4, image->width, image->height);
            break;
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 2, image->width, image->height);
            break;
        case VX_DF_IMAGE_YUV4:
            image->planes = 3;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            vxInitPlane(image, 1, soc, 1, image->width, image->height);
            vxInitPlane(image, 2, soc, 1, image->width, image->height);
            break;
        case VX_DF_IMAGE_IYUV:
            image->planes = 3;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            vxInitPlane(image, 1, soc, 1, image->width/2, image->height/2);
            vxInitPlane(image, 2, soc, 1, image->width/2, image->height/2);
            image->scale[1][VX_DIM_X] = 2;
            image->scale[1][VX_DIM_Y] = 2;
            image->scale[2][VX_DIM_X] = 2;
            image->scale[2][VX_DIM_Y] = 2;
            image->bounds[1][VX_DIM_X][VX_BOUND_END] *= image->scale[1][VX_DIM_X];
            image->bounds[1][VX_DIM_Y][VX_BOUND_END] *= image->scale[1][VX_DIM_Y];
            image->bounds[2][VX_DIM_X][VX_BOUND_END] *= image->scale[2][VX_DIM_X];
            image->bounds[2][VX_DIM_Y][VX_BOUND_END] *= image->scale[2][VX_DIM_Y];
            break;
        case VX_DF_IMAGE_U8:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            break;
        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            break;
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            break;
        case VX_DF_IMAGE_F32:
            image->planes = 1;
            vxInitPlane(image, 0, soc, 1, image->width, image->height);
            break;
        default:
            /*! should not get here unless there's a bug in the
             * vxIsSupportedFourcc call.
             */
            VX_PRINT(VX_ZONE_ERROR, "#################################################\n");
            VX_PRINT(VX_ZONE_ERROR, "Unsupported IMAGE FORMAT!!!\n");
            VX_PRINT(VX_ZONE_ERROR, "#################################################\n");
            break;
    }
    image->memory.nptrs = image->planes;
#if defined(EXPERIMENTAL_USE_OPENCL)
    {
        vx_uint32 i = 0;
        for (i = 0; i < dimof(image_convert_format); i++) {
            if (image_convert_format[i].vx_format == image->format) {
                // copy the structure over if it matches!
                image->memory.cl_type = image_convert_format[i].cl_type;
                memcpy(&image->memory.cl_format, &image_convert_format[i].cl_format, sizeof(cl_image_format));
                break;
            }
        }
    }
#endif
    vxPrintImage(image);
}

void vxFreeImage(vx_image image)
{
    vxFreeMemory(image->base.context, &image->memory);
}

vx_bool vxAllocateImage(vx_image image)
{
    vx_bool ret = vxAllocateMemory(image->base.context, &image->memory);
    vxPrintMemory(&image->memory);
    return ret;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromROI(vx_image image, vx_rectangle_t *rect)
{
    vx_image_t *subimage = NULL;
    if ((vxIsValidImage(image) == vx_true_e) && (rect))
    {
        /* perhaps the parent hasn't been allocated yet? */
        if (vxAllocateMemory(image->base.context, &image->memory) == vx_true_e)
        {
            subimage = (vx_image)vxCreateReference(image->base.context, VX_TYPE_IMAGE, VX_EXTERNAL, &image->base.context->base);
            if (subimage)
            {
                vx_uint32 p = 0;
                /* refer to our parent image and internally refcount it */
                subimage->parent = image;
                vxIncrementReference(&image->base, VX_INTERNAL);
                VX_PRINT(VX_ZONE_IMAGE, "Creating SubImage at {%u,%u},{%u,%u}\n",
                        rect->start_x, rect->start_y, rect->end_x, rect->end_y);
                vxPrintImage(image);
                /* duplicate the metadata */
                subimage->format = image->format;
                subimage->import_type = image->import_type;
                subimage->range = image->range;
                subimage->space = image->space;
                subimage->width = rect->end_x - rect->start_x;
                subimage->height = rect->end_y - rect->start_y;
                subimage->planes = image->planes;
                memcpy(&subimage->scale, &image->scale, sizeof(image->scale));
                memcpy(&subimage->memory, &image->memory, sizeof(image->memory));
                /* modify the dimensions */
                for (p = 0; p < subimage->planes; p++)
                {
                    vx_uint32 offset = vxComputePlaneOffset(image, rect->start_x, rect->start_y, p);
                    VX_PRINT(VX_ZONE_IMAGE, "Offsetting SubImage plane[%u] by %u bytes!\n", p, offset);
                    subimage->memory.dims[p][VX_DIM_X] = subimage->width;
                    subimage->memory.dims[p][VX_DIM_Y] = subimage->height;
                    subimage->memory.ptrs[p] = &image->memory.ptrs[p][offset];
                }
                vxPrintImage(subimage);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Child image failed to allocate!\n");
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Parent image failed to allocate!\n");
        }
    }
    return (vx_image)subimage;
}

static vx_bool vxIsOdd(vx_uint32 a)
{
    if (a & 0x1)
        return vx_true_e;
    else
        return vx_false_e;
}

vx_bool vxIsValidDimensions(vx_uint32 width, vx_uint32 height, vx_df_image color)
{
    if (vxIsOdd(width) && (color == VX_DF_IMAGE_UYVY || color == VX_DF_IMAGE_YUYV))
    {
        return vx_false_e;
    }
    else if ((vxIsOdd(width) || vxIsOdd(height)) &&
              (color == VX_DF_IMAGE_IYUV || color == VX_DF_IMAGE_NV12 || color == VX_DF_IMAGE_NV21))
    {
        return vx_false_e;
    }
    return vx_true_e;
}

static vx_image_t *vxCreateImageInt(vx_context_t *context,
                                     vx_uint32 width,
                                     vx_uint32 height,
                                     vx_df_image color,
                                     vx_bool is_virtual)
{
    vx_image image = NULL;

    if ((vxIsValidContext(context) == vx_true_e) &&
        (vxIsSupportedFourcc(color) == vx_true_e))
    {
        if (vxIsValidDimensions(width, height, color) == vx_true_e)
        {
            image = (vx_image)vxCreateReference(context, VX_TYPE_IMAGE, VX_EXTERNAL, &context->base);
            if (image && image->base.type == VX_TYPE_IMAGE)
            {
                image->base.is_virtual = is_virtual;
                vxInitImage(image, width, height, color);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Requested Image Dimensions are invalid!\n");
            vxAddLogEntry((vx_reference)image, VX_ERROR_INVALID_DIMENSION, "Requested Image Dimensions was invalid!\n");
            image = (vx_image_t *)vxGetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Requested Image Format was invalid!\n");
        vxAddLogEntry((vx_reference)context, VX_ERROR_INVALID_FORMAT, "Requested Image Format was invalid!\n");
        image = (vx_image_t *)vxGetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }
    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    if ((width == 0) || (height == 0) ||
        (vxIsSupportedFourcc(format) == vx_false_e) || (format == VX_DF_IMAGE_VIRT))
    {
        return (vx_image)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }
    return (vx_image)vxCreateImageInt(context, width, height, format, vx_false_e);
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateUniformImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, void *value)
{
    vx_image image = 0;
    if (value == NULL)
        return image;
    image = vxCreateImage(context, width, height, format);
    if (image)
    {
        vx_uint32 x, y, p;
        vx_size planes = 0;
        vx_rectangle_t rect = {0, 0, width, height};
        vxQueryImage(image, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
        for (p = 0; p < planes; p++)
        {
            vx_imagepatch_addressing_t addr;
            void *base = NULL;
            if (vxAccessImagePatch(image, &rect, p, &addr, &base, VX_WRITE_ONLY) == VX_SUCCESS)
            {
                vxPrintImageAddressing(&addr);
                for (y = 0; y < addr.dim_y; y+=addr.step_y)
                {
                    for (x = 0; x < addr.dim_x; x+=addr.step_x)
                    {
                        if (format == VX_DF_IMAGE_U8)
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = *pixel;
                        }
                        else if (format == VX_DF_IMAGE_U16)
                        {
                            vx_uint16 *pixel = (vx_uint16 *)value;
                            vx_uint16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = *pixel;
                        }
                        else if (format == VX_DF_IMAGE_U32)
                        {
                            vx_uint32 *pixel = (vx_uint32 *)value;
                            vx_uint32 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = *pixel;
                        }
                        else if (format == VX_DF_IMAGE_S16)
                        {
                            vx_int16 *pixel = (vx_int16 *)value;
                            vx_int16 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = *pixel;
                        }
                        else if (format == VX_DF_IMAGE_S32)
                        {
                            vx_int32 *pixel = (vx_int32 *)value;
                            vx_int32 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = *pixel;
                        }
                        else if ((format == VX_DF_IMAGE_RGB)  ||
                                 (format == VX_DF_IMAGE_RGBX))
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            ptr[0] = pixel[0];
                            ptr[1] = pixel[1];
                            ptr[2] = pixel[2];
                            if (format == VX_DF_IMAGE_RGBX)
                                ptr[3] = pixel[3];
                        }
                        else if ((format == VX_DF_IMAGE_YUV4) ||
                                 (format == VX_DF_IMAGE_IYUV))
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = pixel[p];
                        }
                        else if ((p == 0) &&
                                 ((format == VX_DF_IMAGE_NV12) ||
                                  (format == VX_DF_IMAGE_NV21)))
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            *ptr = pixel[0];
                        }
                        else if ((p == 1) && (format == VX_DF_IMAGE_NV12))
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            ptr[0] = pixel[1];
                            ptr[1] = pixel[2];
                        }
                        else if ((p == 1) && (format == VX_DF_IMAGE_NV21))
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            ptr[0] = pixel[2];
                            ptr[1] = pixel[1];
                        }
                        else if (format == VX_DF_IMAGE_UYVY)
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            if (x % 2 == 0)
                            {
                                ptr[0] = pixel[1];
                                ptr[1] = pixel[0];
                            }
                            else
                            {
                                ptr[0] = pixel[2];
                                ptr[1] = pixel[0];
                            }
                        }
                        else if (format == VX_DF_IMAGE_YUYV)
                        {
                            vx_uint8 *pixel = (vx_uint8 *)value;
                            vx_uint8 *ptr = vxFormatImagePatchAddress2d(base, x, y, &addr);
                            if (x % 2 == 0)
                            {
                                ptr[0] = pixel[0];
                                ptr[1] = pixel[1];
                            }
                            else
                            {
                                ptr[0] = pixel[0];
                                ptr[1] = pixel[2];
                            }
                        }
                    }
                }
                if (vxCommitImagePatch(image, &rect, p, &addr, base) != VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to set initial image patch on plane %u on const image!\n", p);
                    vxReleaseImage(&image);
                    image = 0;
                    break;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to get image patch on plane %u in const image!\n",p);
                vxReleaseImage(&image);
                image = 0;
                break;
            }
        } /* for loop */
        if (image)
        {
            /* lock the image from being modified again! */
            ((vx_image_t *)image)->constant = vx_true_e;
        }
    }
    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateVirtualImage(vx_graph graph, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_image image = NULL;
    vx_reference_t *gref = (vx_reference_t *)graph;
    if (vxIsValidSpecificReference(gref, VX_TYPE_GRAPH) == vx_true_e)
    {
        image = vxCreateImageInt(gref->context, width, height, format, vx_true_e);
        if (image && image->base.type == VX_TYPE_IMAGE)
        {
            image->base.scope = (vx_reference_t *)graph;
        }
    }
    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromHandle(vx_context context, vx_df_image color, vx_imagepatch_addressing_t addrs[], void *ptrs[], vx_enum import_type)
{
    vx_image image = 0;

    if (vxIsValidImport(import_type) == vx_false_e)
        return 0;

    image = vxCreateImage(context, addrs[0].dim_x, addrs[0].dim_y, color);
    if (image && image->base.type == VX_TYPE_IMAGE)
    {
        vx_uint32 p = 0;
        image->import_type = import_type;
        image->memory.allocated = vx_true_e; /* don't let the system realloc this memory */
        /* now assign the plane pointers, assume linearity */
        for (p = 0; p < image->planes; p++)
        {
            image->memory.ptrs[p] = ptrs[p];
            image->memory.strides[p][VX_DIM_C] = (vx_uint32)vxSizeOfChannel(color);
            image->memory.strides[p][VX_DIM_X] = addrs[p].stride_x;
            image->memory.strides[p][VX_DIM_Y] = addrs[p].stride_y;
            vxCreateSem(&image->memory.locks[p], 1);
        }
    }
    return image;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryImage(vx_image image, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidImage(image) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_IMAGE_ATTRIBUTE_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3))
                {
                    *(vx_df_image *)ptr = image->format;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = image->width;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = image->height;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_PLANES:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = image->planes;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_SPACE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = image->space;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = image->range;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    vx_size size = 0ul;
                    vx_uint32 p;
                    for (p = 0; p < image->planes; p++)
                    {
                        size += (abs(image->memory.strides[p][VX_DIM_Y]) * image->memory.dims[p][VX_DIM_Y]);
                    }
                    *(vx_size *)ptr = size;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    VX_PRINT(VX_ZONE_API, "%s returned %d\n", __FUNCTION__, status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImageAttribute(vx_image image, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidImage(image) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_IMAGE_ATTRIBUTE_SPACE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    image->space = *(vx_enum *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_IMAGE_ATTRIBUTE_RANGE:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3))
                {
                    image->range = *(vx_enum *)ptr;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    VX_PRINT(VX_ZONE_API, "%s returned %d\n", __FUNCTION__, status);
    return status;
}

void vxDestructImage(vx_reference ref)
{
    vx_image image = (vx_image)ref;
    /* if it's not imported and does not have a parent, free it */
    if ((image->import_type == VX_IMPORT_TYPE_NONE) && (image->parent == NULL))
    {
        vxFreeImage(image);
    }
    else if (image->parent)
    {
        vxReleaseReferenceInt((vx_reference *)&image->parent, VX_TYPE_IMAGE, VX_INTERNAL, NULL);
    }
    else if (image->import_type != VX_IMPORT_TYPE_NONE)
    {
        vx_int32 p = 0u;
        for (p = 0; p < image->planes; p++)
        {
            vxDestroySem(&image->memory.locks[p]);
            image->memory.ptrs[p] = NULL;
            image->memory.strides[p][VX_DIM_C] = 0;
            image->memory.strides[p][VX_DIM_X] = 0;
            image->memory.strides[p][VX_DIM_Y] = 0;
        }
        image->memory.allocated = vx_false_e;
    }
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseImage(vx_image *image)
{
    return vxReleaseReferenceInt((vx_reference *)image, VX_TYPE_IMAGE, VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_size VX_API_CALL vxComputeImagePatchSize(vx_image image,
                                       vx_rectangle_t *rect,
                                       vx_uint32 plane_index)
{
    vx_size size = 0ul;
    vx_uint32 start_x = 0u, start_y = 0u, end_x = 0u, end_y = 0u;

    if ((vxIsValidImage(image) == vx_true_e) && (rect))
    {
        start_x = rect->start_x;
        start_y = rect->start_y;
        end_x = rect->end_x;
        end_y = rect->end_y;

        if (image->memory.ptrs[0] == NULL)
        {
            if (vxAllocateImage(image) == vx_false_e)
            {
                vxAddLogEntry((vx_reference)image, VX_ERROR_NO_MEMORY, "Failed to allocate image!\n");
                return 0;
            }
        }
        if (plane_index < image->planes)
        {
            vx_size numPixels = ((end_x-start_x)/image->scale[plane_index][VX_DIM_X]) *
                                ((end_y-start_y)/image->scale[plane_index][VX_DIM_Y]);
            vx_size pixelSize = image->memory.strides[plane_index][VX_DIM_X];
            vxPrintImage(image);
            VX_PRINT(VX_ZONE_IMAGE, "numPixels = "VX_FMT_SIZE" pixelSize = "VX_FMT_SIZE"\n", numPixels, pixelSize);
            size = numPixels * pixelSize;
        }
        else
        {
            vxAddLogEntry((vx_reference)image, VX_ERROR_INVALID_PARAMETERS, "Plane index %u is out of bounds!", plane_index);
        }

        VX_PRINT(VX_ZONE_IMAGE, "image %p for patch {%u,%u to %u,%u} has a byte size of "VX_FMT_SIZE"\n",
                 image, start_x, start_y, end_x, end_y, size);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Image Reference is invalid!\n");
    }
    return size;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessImagePatch(vx_image image,
                                    vx_rectangle_t *rect,
                                    vx_uint32 plane_index,
                                    vx_imagepatch_addressing_t *addr,
                                    void **ptr,
                                    vx_enum usage)
{
    vx_uint8 *p = NULL;
    vx_status status = VX_FAILURE;
    vx_bool mapped = vx_false_e;
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x = rect ? rect->end_x : 0u;
    vx_uint32 end_y = rect ? rect->end_y : 0u;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (addr == NULL) || (ptr == NULL))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* bad references */
    if ((!rect) ||
        (vxIsValidImage(image) == vx_false_e))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* determine if virtual before checking for memory */
    if (image->base.is_virtual == vx_true_e)
    {
        if (image->base.is_accessible == vx_false_e)
        {
            /* User tried to access a "virtual" image. */
            VX_PRINT(VX_ZONE_ERROR, "Can not access a virtual image\n");
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    /* more bad parameters */
    if (zero_area == vx_false_e &&
        ((plane_index >= (vx_uint32)abs(image->memory.nptrs)) ||
         (plane_index >= image->planes) ||
         (rect->start_x >= rect->end_x) ||
         (rect->start_y >= rect->end_y)))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* verify has not been call yet or will not be called (immediate mode)
     * this is a trick to "touch" an image so that it can be created
     */
    if ((image->memory.ptrs[0] == NULL) && (vxAllocateImage(image) == vx_false_e))
    {
        VX_PRINT(VX_ZONE_ERROR, "No memory!\n");
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    /* can't write to constant */
    if ((image->constant == vx_true_e) && ((usage == VX_WRITE_ONLY) ||
                                         (usage == VX_READ_AND_WRITE)))
    {
        status = VX_ERROR_NOT_SUPPORTED;
        VX_PRINT(VX_ZONE_ERROR, "Can't write to constant data, only read!\n");
        vxAddLogEntry(&image->base, status, "Can't write to constant data, only read!\n");
        goto exit;
    }

    /*************************************************************************/
    VX_PRINT(VX_ZONE_IMAGE, "AccessImagePatch from "VX_FMT_REF" to ptr %p from {%u,%u} to {%u,%u} plane %u\n",
            image, *ptr, rect->start_x, rect->start_y, rect->end_x, rect->end_y, plane_index);

    /* POSSIBILITIES:
     * 1.) !*ptr && RO == MAP
     * 2.) !*ptr && WO == MAP
     * 3.) !*ptr && RW == MAP
     * 4.)  *ptr && RO||RW == COPY (UNLESS MAP)
     */

    if ((*ptr == NULL) && ((usage == VX_READ_ONLY) ||
                           (usage == VX_WRITE_ONLY) ||
                           (usage == VX_READ_AND_WRITE)))
    {
        mapped = vx_true_e;
    }

    if (mapped == vx_true_e)
    {
        vx_uint32 index = 0u;

        /* lock the memory plane for multiple writers*/
        if (usage != VX_READ_ONLY) {
            if (vxSemWait(&image->memory.locks[plane_index]) == vx_false_e) {
                status = VX_ERROR_NO_RESOURCES;
                goto exit;
            }
        }
        vxPrintMemory(&image->memory);
        p = (vx_uint8 *)image->memory.ptrs[plane_index];
        /* use the addressing of the internal format */
        addr->dim_x = rect->end_x - rect->start_x;
        addr->dim_y = rect->end_y - rect->start_y;
        addr->stride_x = image->memory.strides[plane_index][VX_DIM_X];
        addr->stride_y = image->memory.strides[plane_index][VX_DIM_Y];
        addr->step_x = image->scale[plane_index][VX_DIM_X];
        addr->step_y = image->scale[plane_index][VX_DIM_Y];
        addr->scale_x = VX_SCALE_UNITY / image->scale[plane_index][VX_DIM_X];
        addr->scale_y = VX_SCALE_UNITY / image->scale[plane_index][VX_DIM_Y];
        index = vxComputePatchOffset(rect->start_x, rect->start_y, addr);
        *ptr = &p[index];
        VX_PRINT(VX_ZONE_IMAGE, "Returning mapped pointer %p which is offset by %lu\n", *ptr, index);
        vxReadFromReference(&image->base);
        vxIncrementReference(&image->base, VX_EXTERNAL);
        status = VX_SUCCESS;
    }
    else
    {
        vx_size size = vxComputeImagePatchSize(image, rect, plane_index);
        vx_uint32 a = 0u;
        if (vxAddAccessor(image->base.context, size, usage, *ptr, &image->base, &a) == vx_true_e)
        {
            *ptr = image->base.context->accessors[a].ptr;
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
            vxAddLogEntry(&image->base, status, "Failed to allocate memory for COPY-ON-READ! Size="VX_FMT_SIZE"\n", size);
            goto exit;
        }
    }

    if (*ptr != NULL && mapped == vx_false_e)
    {
        vx_uint32 y, i, j, len;
        vx_uint8 *tmp = *ptr;

        /*! \todo implement overlapping multi-writers lock, not just single writer lock */
        if ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE))
        {
            if (vxSemWait(&image->memory.locks[plane_index]) == vx_false_e)
            {
                status = VX_ERROR_NO_RESOURCES;
                goto exit;
            }
        }
        /* use the dimensionality of a flat buffer. */
        addr->dim_x = rect->end_x - rect->start_x;
        addr->dim_y = rect->end_y - rect->start_y;
        addr->stride_x = image->memory.strides[plane_index][VX_DIM_X];
        addr->stride_y = addr->dim_x / image->scale[plane_index][VX_DIM_X] * addr->stride_x;
        addr->step_x = image->scale[plane_index][VX_DIM_X];
        addr->step_y = image->scale[plane_index][VX_DIM_Y];
        addr->scale_x = VX_SCALE_UNITY / image->scale[plane_index][VX_DIM_X];
        addr->scale_y = VX_SCALE_UNITY / image->scale[plane_index][VX_DIM_Y];
        if ((usage == VX_READ_ONLY) || (usage == VX_READ_AND_WRITE))
        {
            for (y = rect->start_y; y < rect->end_y; y+=addr->step_y)
            {
                i = vxComputePlaneOffset(image, rect->start_x, y, plane_index);
                j = vxComputePatchOffset(0, (y - rect->start_y), addr);
                len = vxComputePlaneRangeSize(image, addr->dim_x, plane_index);
                VX_PRINT(VX_ZONE_IMAGE, "%p[%u] <= %p[%u] for %u\n", tmp, j, image->memory.ptrs[plane_index], i, len);
                memcpy(&tmp[j], &image->memory.ptrs[plane_index][i], len);
            }
            VX_PRINT(VX_ZONE_IMAGE, "Copied image into %p\n", *ptr);
            vxReadFromReference(&image->base);
        }
        vxIncrementReference(&image->base, VX_EXTERNAL);
        status = VX_SUCCESS;
    }
exit:
    VX_PRINT(VX_ZONE_API, "returned %d\n", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitImagePatch(vx_image image,
                                    vx_rectangle_t *rect,
                                    vx_uint32 plane_index,
                                    vx_imagepatch_addressing_t *addr,
                                    void *ptr)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_int32 i = 0;
    vx_bool external = vx_true_e; // assume that it was an allocated buffer
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x = rect ? rect->end_x : 0u;
    vx_uint32 end_y = rect ? rect->end_y : 0u;
    vx_uint8 *tmp = (vx_uint8 *)ptr;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);
    vx_uint32 index = UINT32_MAX; // out of bounds, if given to remove, won't do anything

    if (vxIsValidImage(image) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    VX_PRINT(VX_ZONE_IMAGE, "CommitImagePatch to "VX_FMT_REF" from ptr %p plane %u to {%u,%u},{%u,%u}\n",
        image, ptr, plane_index, start_x, start_y, end_x, end_y);

    vxPrintImage(image);
    vxPrintImageAddressing(addr);

    /* determine if virtual before checking for memory */
    if (image->base.is_virtual == vx_true_e && zero_area == vx_false_e)
    {
        if (image->base.is_accessible == vx_false_e)
        {
            /* User tried to access a "virtual" image. */
            VX_PRINT(VX_ZONE_ERROR, "Can not access a virtual image\n");
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    if (zero_area == vx_false_e &&
        ((plane_index >= image->planes) ||
         (plane_index >= (vx_uint32)abs(image->memory.nptrs)) ||
         (ptr == NULL) ||
         (addr == NULL)))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* check the rectangle, it has to be in actual plane space */
    if (zero_area == vx_false_e &&
        ((start_x >= end_x) || ((end_x - start_x) > addr->dim_x) ||
        (start_y >= end_y) || ((end_y - start_y) > addr->dim_y) ||
        (end_x > (vx_uint32)image->memory.dims[plane_index][VX_DIM_X] * (vx_uint32)image->scale[plane_index][VX_DIM_X]) ||
        (end_y > (vx_uint32)image->memory.dims[plane_index][VX_DIM_Y] * (vx_uint32)image->scale[plane_index][VX_DIM_X])))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid start,end coordinates! plane %u {%u,%u},{%u,%u}\n",
                 plane_index, start_x, start_y, end_x, end_y);
        vxPrintImage(image);
        DEBUG_BREAK();
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    {
        /* VARIABLES:
         * 1.) ZERO_AREA
         * 2.) CONSTANT - independant
         * 3.) INTERNAL - independant of area
         * 4.) EXTERNAL - dependant on area (do nothing on zero, determine on non-zero)
         * 5.) !INTERNAL && !EXTERNAL == MAPPED
         */
        vx_bool internal = vxFindAccessor(image->base.context, ptr, &index);

        if ((zero_area == vx_false_e) && (image->constant == vx_true_e))
        {
            /* we tried to modify constant data! */
            VX_PRINT(VX_ZONE_ERROR, "Can't set constant image data!\n");
            status = VX_ERROR_NOT_SUPPORTED;
            /* don't modify the accessor here, it's an error case */
            goto exit;
        }
        else if (zero_area == vx_false_e && image->constant == vx_false_e)
        {
            /* this could be a write-back */
            if (internal == vx_true_e && image->base.context->accessors[index].usage == VX_READ_ONLY)
            {
                /* this is a buffer that we allocated on behalf of the user and now they are done. Do nothing else*/
                vxRemoveAccessor(image->base.context, index);
            }
            else
            {
                /* determine if this grows the valid region */
                if (image->region.start_x > start_x)
                    image->region.start_x = start_x;
                if (image->region.start_y > start_y)
                    image->region.start_y = start_y;
                if (image->region.end_x < end_x)
                    image->region.end_x = end_x;
                if (image->region.end_y < end_y)
                    image->region.end_y = end_y;

                /* index of 1 pixel line past last. */
                i = (image->memory.dims[plane_index][VX_DIM_Y] * image->memory.strides[plane_index][VX_DIM_Y]);

                VX_PRINT(VX_ZONE_IMAGE, "base:%p tmp:%p end:%p\n",
                    image->memory.ptrs[plane_index], ptr, &image->memory.ptrs[plane_index][i]);

                if ((image->memory.ptrs[plane_index] <= (vx_uint8 *)ptr) &&
                    ((vx_uint8 *)ptr < &image->memory.ptrs[plane_index][i]))
                {
                    /* corner case for 2d memory */
                    if (image->memory.strides[plane_index][VX_DIM_Y] !=
                        (image->memory.dims[plane_index][VX_DIM_X] * image->memory.strides[plane_index][VX_DIM_X]))
                    {
                        /* determine if the pointer is within the image boundary. */
                        vx_uint8 *base = image->memory.ptrs[plane_index];
                        vx_size offset = ((vx_size)(tmp - base)) % image->memory.strides[plane_index][VX_DIM_Y];
                        if (offset < (image->memory.dims[plane_index][VX_DIM_X] * image->memory.strides[plane_index][VX_DIM_X]))
                        {
                            VX_PRINT(VX_ZONE_IMAGE, "Pointer is within 2D image\n");
                            external = vx_false_e;
                        }
                    }
                    else
                    {
                        /* the pointer in contained in the image, so it was mapped, thus
                         * there's nothing else to do. */
                        external = vx_false_e;
                        VX_PRINT(VX_ZONE_IMAGE, "Mapped pointer detected!\n");
                    }
                }
                if (external == vx_true_e || internal == vx_true_e)
                {
                    /* copy the patch back to the image. */
                    vx_uint32 y, i, j, len;
                    for (y = start_y; y < end_y; y += addr->step_y)
                    {
                        i = vxComputePlaneOffset(image, start_x, y, plane_index);
                        j = vxComputePatchOffset(0, (y - start_y), addr);
                        len = vxComputePatchRangeSize((end_x - start_x), addr);
                        VX_PRINT(VX_ZONE_IMAGE, "%p[%u] <= %p[%u] for %u\n", image->memory.ptrs[plane_index], j, tmp, i, len);
                        memcpy(&image->memory.ptrs[plane_index][i], &tmp[j], len);
                    }
                    if (internal == vx_true_e)
                    {
                        /* a write only or read/write copy */
                        vxRemoveAccessor(image->base.context, index);
                    }
                }
                vxWroteToReference(&image->base);
            }
            status = VX_SUCCESS;
            vxSemPost(&image->memory.locks[plane_index]);
        }
        else if (zero_area == vx_true_e)
        {
            /* could be RO|WO|RW where they decided not to commit anything. */
            if (internal == vx_true_e) // RO
            {
                vxRemoveAccessor(image->base.context, index);
            }
            else // RW|WO
            {
                /*! \bug (possible bug, but maybe not) anyone can decrement an
                 *  image access, should we limit to incrementor? that would be
                 *  a lot to track */
                vxSemPost(&image->memory.locks[plane_index]);
            }
            status = VX_SUCCESS;
        }
        VX_PRINT(VX_ZONE_IMAGE, "Decrementing Image Reference\n");
        vxDecrementReference(&image->base, VX_EXTERNAL);
    }
exit:
    VX_PRINT(VX_ZONE_API, "return %d\n", status);
    return status;
}

VX_API_ENTRY void * VX_API_CALL vxFormatImagePatchAddress1d(void *ptr, vx_uint32 index, vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if (ptr && index < addr->dim_x*addr->dim_y)
    {
        vx_uint32 x = index % addr->dim_x;
        vx_uint32 y = index / addr->dim_x;
        vx_uint32 offset = vxComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}

VX_API_ENTRY void * VX_API_CALL vxFormatImagePatchAddress2d(void *ptr, vx_uint32 x, vx_uint32 y, vx_imagepatch_addressing_t *addr)
{
    vx_uint8 *new_ptr = NULL;
    if (ptr && x < addr->dim_x && y < addr->dim_y)
    {
        vx_uint32 offset = vxComputePatchOffset(x, y, addr);
        new_ptr = (vx_uint8 *)ptr;
        new_ptr = &new_ptr[offset];
    }
    return new_ptr;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetValidRegionImage(vx_image image, vx_rectangle_t *rect)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxIsValidImage(image) == vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        if (rect)
        {
            if ((image->region.start_x <= image->region.end_x) && (image->region.start_y <= image->region.end_y))
            {
                rect->start_x = image->region.start_x;
                rect->start_y = image->region.start_y;
                rect->end_x = image->region.end_x;
                rect->end_y = image->region.end_y;
            }
            else
            {
                rect->start_x = 0;
                rect->start_y = 0;
                rect->end_x = image->width;
                rect->end_y = image->height;
            }
            status = VX_SUCCESS;
        }
    }
    return status;
}

void vxPrintImage(vx_image image)
{
    vx_uint32 p = 0;
    vx_char df_image[5];
    strncpy(df_image, (char *)&image->format, 4);
    df_image[4] = '\0';
    vxPrintReference(&image->base);
    VX_PRINT(VX_ZONE_IMAGE, "vx_image_t:%p %s %ux%u (%s)\n", image, df_image, image->width, image->height, (image->constant?"CONSTANT":"MUTABLE"));
    for (p = 0; p < image->planes; p++)
    {
        VX_PRINT(VX_ZONE_IMAGE, "\tplane[%u] ptr:%p dim={%u,%u,%u} stride={%u,%u,%u} scale={%u,%u,%u} bounds={%u,%ux%u,%u}\n",
                     p,
                     image->memory.ptrs[p],
                     image->memory.dims[p][VX_DIM_C],
                     image->memory.dims[p][VX_DIM_X],
                     image->memory.dims[p][VX_DIM_Y],
                     image->memory.strides[p][VX_DIM_C],
                     image->memory.strides[p][VX_DIM_X],
                     image->memory.strides[p][VX_DIM_Y],
                     image->scale[p][VX_DIM_C],
                     image->scale[p][VX_DIM_X],
                     image->scale[p][VX_DIM_Y],
                     image->bounds[p][VX_DIM_X][VX_BOUND_START],
                     image->bounds[p][VX_DIM_X][VX_BOUND_END],
                     image->bounds[p][VX_DIM_Y][VX_BOUND_START],
                     image->bounds[p][VX_DIM_Y][VX_BOUND_END]);
    }
}
