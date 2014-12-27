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

#include <VX/vx.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

vx_status vx_example_extensions(vx_context context) {
//! [extensions]
    vx_char *tmp, *extensions = NULL;
    vx_size size = 0ul;
    vxQueryContext(context,VX_CONTEXT_ATTRIBUTE_EXTENSIONS_SIZE,&size,sizeof(size));
    extensions = malloc(size);
    vxQueryContext(context,VX_CONTEXT_ATTRIBUTE_EXTENSIONS,
                   extensions, size);
//! [extensions]
    tmp = strtok(extensions, " ");
    do {
        if (tmp)
            printf("Extension: %s\n", tmp);
        tmp = strtok(NULL, " ");
    } while (tmp);
    free(extensions);
    return VX_SUCCESS;
}
