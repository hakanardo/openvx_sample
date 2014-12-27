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
#include <VX/vx_lib_debug.h>
#include <debug_k.h>
#include <stdio.h>

vx_status vxFWriteImage(vx_image input, vx_array file)
{
    vx_char *filename = NULL;
    vx_size filename_stride = 0;
    vx_uint8 *src[4] = {NULL, NULL, NULL, NULL};
    vx_uint32 p, y, sx, ex, sy, ey, width, height;
    vx_size planes;
    vx_imagepatch_addressing_t addr[4];
    vx_df_image format;
    FILE *fp = NULL;
    vx_char *ext = NULL;
    size_t wrote = 0ul;
    vx_rectangle_t rect;

    vx_status status = vxAccessArrayRange(file, 0, VX_MAX_FILE_NAME, &filename_stride, (void **)&filename, VX_READ_ONLY);
    if (status != VX_SUCCESS || filename_stride != sizeof(vx_char))
    {
        vxCommitArrayRange(file, 0, 0, filename);
        vxAddLogEntry((vx_reference)file, VX_FAILURE, "Incorrect array "VX_FMT_REF"\n", file);
        return VX_FAILURE;
    }
    //VX_PRINT(VX_ZONE_INFO, "filename=%s\n",filename);
    fp = fopen(filename, "wb+");
    if (fp == NULL) {
        vxCommitArrayRange(file, 0, 0, filename);
        vxAddLogEntry((vx_reference)file, VX_FAILURE, "Failed to open file %s\n",filename);
        return VX_FAILURE;
    }

    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH,  &width,  sizeof(width));
    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
    status |= vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

    status |= vxGetValidRegionImage(input, &rect);

    sx = rect.start_x;
    sy = rect.start_y;
    ex = rect.end_x;
    ey = rect.end_y;

    ext = strrchr(filename, '.');
    if (ext && (strcmp(ext, ".pgm") == 0 || strcmp(ext, ".PGM") == 0))
    {
        fprintf(fp, "P5\n# %s\n",filename);
        fprintf(fp, "%u %u\n", width, height);
        if (format == VX_DF_IMAGE_U8)
            fprintf(fp, "255\n");
        else if (format == VX_DF_IMAGE_S16)
            fprintf(fp, "65535\n");
        else if (format == VX_DF_IMAGE_U16)
            fprintf(fp, "65535\n");
    }
    for (p = 0u; p < planes; p++)
    {
        status |= vxAccessImagePatch(input, &rect, p, &addr[p], (void **)&src[p], VX_READ_ONLY);
    }
    for (p = 0u; (p < planes) && (status == VX_SUCCESS); p++)
    {
        size_t len = addr[p].stride_x * (addr[p].dim_x * addr[p].scale_x)/VX_SCALE_UNITY;
        for (y = 0u; y < height; y+=addr[p].step_y)
        {
            vx_uint32 i = 0;
            vx_uint8 *ptr = NULL;
            uint8_t value = 0u;

            if (y < sy || y >= ey)
            {
                for (i = 0; i < width; ++i) {
                    wrote += fwrite(&value, sizeof(value), 1, fp);
                }
                continue;
            }

            for (i = 0; i < sx; ++i)
                wrote += fwrite(&value, sizeof(value), 1, fp);

            ptr = vxFormatImagePatchAddress2d(src[p], 0, y - sy, &addr[p]);
            wrote += fwrite(ptr, 1, len, fp);

            for (i = 0; i < width - ex; ++i)
                wrote += fwrite(&value, sizeof(value), 1, fp);
        }
        if (wrote == 0)
        {
            vxAddLogEntry((vx_reference)file, VX_FAILURE, "Failed to write to file!\n");
            status = VX_FAILURE;
            break;
        }
        if (status == VX_FAILURE)
        {
            vxAddLogEntry((vx_reference)file, VX_FAILURE, "Failed to write image to file correctly\n");
            break;
        }
    }
    for (p = 0u; p < planes; p++)
    {
        status |= vxCommitImagePatch(input, NULL, p, &addr[p], src[p]);
    }
    if (status != VX_SUCCESS)
    {
        vxAddLogEntry((vx_reference)file, VX_FAILURE, "Failed to write image to file correctly\n");
    }
    fflush(fp);
    fclose(fp);
    if (vxCommitArrayRange(file, 0, 0, filename) != VX_SUCCESS)
    {
        vxAddLogEntry((vx_reference)file, VX_FAILURE, "Failed to release handle to filename array!\n");
    }

    return status;
}

vx_status vxFReadImage(vx_array file, vx_image output)
{
    vx_char *filename = NULL;
    vx_size filename_stride = 0;
    vx_uint8 *src = NULL;
    vx_uint32 p = 0u, y = 0u, planes = 0u;
    vx_imagepatch_addressing_t addr = {0};
    vx_df_image format = VX_DF_IMAGE_VIRT;
    FILE *fp = NULL;
    vx_char tmp[VX_MAX_FILE_NAME] = {0};
    vx_char *ext = NULL;
    vx_rectangle_t rect;
    vx_uint32 width = 0, height = 0;

    vx_status status = vxAccessArrayRange(file, 0, VX_MAX_FILE_NAME, &filename_stride, (void **)&filename, VX_READ_ONLY);
    if (status != VX_SUCCESS || filename_stride != sizeof(vx_char))
    {
        vxAddLogEntry((vx_reference)file, VX_FAILURE, "Incorrect array "VX_FMT_REF"\n", file);
        return VX_FAILURE;
    }
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        vxAddLogEntry((vx_reference)file, VX_FAILURE, "Failed to open file %s\n",filename);
        return VX_FAILURE;
    }

    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_PLANES, &planes, sizeof(planes));
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

    ext = strrchr(filename, '.');
    if (ext && (strcmp(ext, ".pgm") == 0 || strcmp(ext, ".PGM") == 0))
    {
        FGETS(tmp, fp); // PX
        FGETS(tmp, fp); // comment
        FGETS(tmp, fp); // W H
        sscanf(tmp, "%u %u", &width, &height);
        FGETS(tmp, fp); // BPP
        // ! \todo double check image size?
    }
    else if (ext && (strcmp(ext, ".yuv") == 0 ||
                     strcmp(ext, ".rgb") == 0 ||
                     strcmp(ext, ".bw") == 0))
    {
        sscanf(filename, "%*[^_]_%ux%u_%*s", &width, &height);
    }

    rect.start_x = rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    for (p = 0; p < planes; p++)
    {
        status = vxAccessImagePatch(output, &rect, p, &addr, (void **)&src, VX_WRITE_ONLY);
        if (status == VX_SUCCESS)
        {
            for (y = 0; y < addr.dim_y; y+=addr.step_y)
            {
                vx_uint8 *srcp = vxFormatImagePatchAddress2d(src, 0, y, &addr);
                vx_size len = ((addr.dim_x * addr.scale_x)/VX_SCALE_UNITY);
                vx_size rlen = fread(srcp, addr.stride_x, len, fp);
                if (rlen != len)
                {
                    status = VX_FAILURE;
                    break;
                }
            }
            if (status == VX_SUCCESS)
            {
                status = vxCommitImagePatch(output, &rect, p, &addr, src);
            }
            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }
    fclose(fp);
    vxCommitArrayRange(file, 0, 0, filename);

    return status;
}

