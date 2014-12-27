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

void vxDestructPyramid(vx_reference ref)
{
    vx_pyramid pyramid = (vx_pyramid)ref;
    vx_uint32 i = 0;
    for (i = 0; i < pyramid->numLevels; i++)
    {
        vxReleaseReferenceInt((vx_reference *)&pyramid->levels[i], VX_TYPE_IMAGE, VX_INTERNAL, NULL);
    }
    free(pyramid->levels);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleasePyramid(vx_pyramid *pyr)
{
    return vxReleaseReferenceInt((vx_reference *)pyr, VX_TYPE_PYRAMID, VX_EXTERNAL, NULL);
}

vx_status vxInitPyramid(vx_pyramid pyramid,
                        vx_size levels,
                        vx_float32 scale,
                        vx_uint32 width,
                        vx_uint32 height,
                        vx_df_image format)
{
    const vx_float32 c_orbscale[4] = {0.5f, VX_SCALE_PYRAMID_ORB, VX_SCALE_PYRAMID_ORB * VX_SCALE_PYRAMID_ORB, VX_SCALE_PYRAMID_ORB * VX_SCALE_PYRAMID_ORB * VX_SCALE_PYRAMID_ORB};
    vx_status status = VX_SUCCESS;

    /* very first init will come in here */
    if (pyramid->levels == NULL)
    {
        pyramid->numLevels = levels;
        pyramid->scale = scale;
        pyramid->levels = (vx_image *)calloc(levels, sizeof(vx_image_t *));
    }

    /* these could be "virtual" values or hard values */
    pyramid->width = width;
    pyramid->height = height;
    pyramid->format = format;

    if (pyramid->levels)
    {
        if (pyramid->width != 0 && pyramid->height != 0 && format != VX_DF_IMAGE_VIRT)
        {
            vx_int32 i;
            vx_uint32 w = pyramid->width;
            vx_uint32 h = pyramid->height;
            vx_uint32 ref_w = pyramid->width;
            vx_uint32 ref_h = pyramid->height;

            for (i = 0; i < pyramid->numLevels; i++)
            {
                vx_context c = (vx_context)pyramid->base.context;
                if (pyramid->levels[i] == 0)
                {
                    pyramid->levels[i] = vxCreateImage(c, w, h, format);

                    /* increment the internal counter on the image, not the external one */
                    vxIncrementReference((vx_reference_t *)pyramid->levels[i], VX_INTERNAL);
                    vxDecrementReference((vx_reference_t *)pyramid->levels[i], VX_EXTERNAL);

                    /* remember that the scope of the image is the pyramid */
                    ((vx_image_t *)pyramid->levels[i])->base.scope = (vx_reference_t *)pyramid;

                    if (VX_SCALE_PYRAMID_ORB == scale)
                    {
                        vx_float32 orb_scale = c_orbscale[(i + 1) % 4];
                        w = (vx_uint32)ceilf((vx_float32)ref_w * orb_scale);
                        h = (vx_uint32)ceilf((vx_float32)ref_h * orb_scale);
                        if (0 == ((i + 1) % 4))
                        {
                            ref_w = w;
                            ref_h = h;
                        }
                    }
                    else
                    {
                        w = (vx_uint32)ceilf((vx_float32)w * scale);
                        h = (vx_uint32)ceilf((vx_float32)h * scale);
                    }
                }
            }
        }
        else
        {
            /* virtual images, but in a pyramid we really need to know the
             * level 0 value. Dimensionless images don't work after validation
             * time.
             */
        }
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }
    return status;
}

static vx_pyramid vxCreatePyramidInt(vx_context context,
                                      vx_size levels,
                                      vx_float32 scale,
                                      vx_uint32 width,
                                      vx_uint32 height,
                                      vx_df_image format,
                                      vx_bool is_virtual)
{
    vx_pyramid pyramid = NULL;

    if (vxIsValidContext(context) == vx_false_e)
        return NULL;

    if ((scale != VX_SCALE_PYRAMID_HALF) &&
        (scale != VX_SCALE_PYRAMID_ORB))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid scale %lf for pyramid!\n",scale);
        vxAddLogEntry((vx_reference)context, VX_ERROR_INVALID_PARAMETERS, "Invalid scale %lf for pyramid!\n",scale);
        pyramid = (vx_pyramid_t *)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }
    else if (levels == 0 || levels > 8)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid number of levels for pyramid!\n", levels);
        vxAddLogEntry((vx_reference)context, VX_ERROR_INVALID_PARAMETERS, "Invalid number of levels for pyramid!\n", levels);
        pyramid = (vx_pyramid_t *)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }
    else
    {
        pyramid = (vx_pyramid)vxCreateReference(context, VX_TYPE_PYRAMID, VX_EXTERNAL, &context->base);
        if (pyramid && pyramid->base.type == VX_TYPE_PYRAMID)
        {
            vx_status status;
            pyramid->base.is_virtual = is_virtual;
            status = vxInitPyramid(pyramid, levels, scale, width, height, format);
            if (status != VX_SUCCESS)
            {
                vxAddLogEntry((vx_reference)pyramid, status, "Failed to initialize pyramid\n");
                vxReleasePyramid((vx_pyramid *)&pyramid);
                pyramid = (vx_pyramid_t *)vxGetErrorObject(context, status);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate memory\n");
            vxAddLogEntry((vx_reference)context, VX_ERROR_NO_MEMORY, "Failed to allocate memory\n");
            pyramid = (vx_pyramid_t *)vxGetErrorObject(context, VX_ERROR_NO_MEMORY);
        }
    }
    return pyramid;
}

VX_API_ENTRY vx_pyramid VX_API_CALL vxCreateVirtualPyramid(vx_graph graph,
                                         vx_size levels,
                                         vx_float32 scale,
                                         vx_uint32 width,
                                         vx_uint32 height,
                                         vx_df_image format)
{
    vx_pyramid pyramid = NULL;
    if (vxIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_true_e)
    {
        pyramid = vxCreatePyramidInt(graph->base.context, levels, scale,
                                 width, height, format,
                                 vx_true_e);
        if (pyramid && pyramid->base.type == VX_TYPE_PYRAMID)
        {
            pyramid->base.scope = (vx_reference_t *)graph;
        }
    }
    return pyramid;
}

VX_API_ENTRY vx_pyramid VX_API_CALL vxCreatePyramid(vx_context context, vx_size levels, vx_float32 scale, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    if ((width == 0) || (height == 0) || (format == VX_DF_IMAGE_VIRT))
    {
        return (vx_pyramid)vxGetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }
    return (vx_pyramid)vxCreatePyramidInt(context,
                                           levels, scale, width, height, format,
                                           vx_false_e);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryPyramid(vx_pyramid pyramid, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;
    if (vxIsValidSpecificReference(&pyramid->base, VX_TYPE_PYRAMID) == vx_true_e)
    {
        switch (attribute)
        {
            case VX_PYRAMID_ATTRIBUTE_LEVELS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3))
                {
                    *(vx_size *)ptr = pyramid->numLevels;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_ATTRIBUTE_SCALE:
                if (VX_CHECK_PARAM(ptr, size, vx_float32, 0x3))
                {
                    *(vx_float32 *)ptr = pyramid->scale;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_ATTRIBUTE_WIDTH:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = pyramid->width;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_ATTRIBUTE_HEIGHT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3))
                {
                    *(vx_uint32 *)ptr = pyramid->height;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_PYRAMID_ATTRIBUTE_FORMAT:
                if (VX_CHECK_PARAM(ptr, size, vx_df_image, 0x3))
                {
                    *(vx_df_image *)ptr = pyramid->format;
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
    return status;
}

VX_API_ENTRY vx_image VX_API_CALL vxGetPyramidLevel(vx_pyramid pyramid, vx_uint32 index)
{
    vx_image image = 0;
    if (vxIsValidSpecificReference(&pyramid->base, VX_TYPE_PYRAMID) == vx_true_e)
    {
        if (index < pyramid->numLevels)
        {
            image = pyramid->levels[index];
            vxIncrementReference(&image->base, VX_EXTERNAL);
        }
    }
    return image;
}
