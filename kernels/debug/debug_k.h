/*
 * Copyright (c) 2011-2014 The Khronos Group Inc.
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

#ifndef _VX_DEBUG_K_H_
#define _VX_DEBUG_K_H_

#include <VX/vx.h>
#include <VX/vx_helper.h>

#define FGETS(str, fh)                              \
{                                                   \
    char* success = fgets(str, sizeof(str), fh);    \
    if (!success)                                   \
    {                                               \
        printf("fgets failed\n");                   \
    }                                               \
}

#ifdef __cplusplus
extern "C" {
#endif

vx_status vxFWriteImage (vx_image input, vx_array filename);
vx_status vxFReadImage  (vx_array filename, vx_image output);

vx_status vxCopyImage(vx_image input, vx_image output);
vx_status vxCopyArray(vx_array src, vx_array dst);

#ifdef __cplusplus
}
#endif

#endif  // !_VX_DEBUG_K_H_

