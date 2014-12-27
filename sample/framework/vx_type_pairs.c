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

#if defined(EXPERIMENTAL_USE_DOT) || defined(EXPERIMENTAL_USE_XML)

#include <vx_type_pairs.h>

vx_enum_string_t type_pairs[] = {
    {VX_STRINGERIZE(VX_TYPE_INVALID),0},
    /* scalar objects */
    {VX_STRINGERIZE(VX_TYPE_CHAR),sizeof(vx_char)*2},
    {VX_STRINGERIZE(VX_TYPE_UINT8),sizeof(vx_uint8)*2},
    {VX_STRINGERIZE(VX_TYPE_UINT16),sizeof(vx_uint16)*2},
    {VX_STRINGERIZE(VX_TYPE_UINT32),sizeof(vx_uint32)*2},
    {VX_STRINGERIZE(VX_TYPE_UINT64),sizeof(vx_uint64)*2},
    {VX_STRINGERIZE(VX_TYPE_INT8),sizeof(vx_int8)*2},
    {VX_STRINGERIZE(VX_TYPE_INT16),sizeof(vx_int16)*2},
    {VX_STRINGERIZE(VX_TYPE_INT32),sizeof(vx_int32)*2},
    {VX_STRINGERIZE(VX_TYPE_INT64),sizeof(vx_int64)*2},
    {VX_STRINGERIZE(VX_TYPE_FLOAT32),sizeof(vx_float32)*2},
    {VX_STRINGERIZE(VX_TYPE_FLOAT64),sizeof(vx_float64)*2},
    {VX_STRINGERIZE(VX_TYPE_SIZE),sizeof(vx_size)*2},
    {VX_STRINGERIZE(VX_TYPE_DF_IMAGE),sizeof(vx_df_image)*2},
    {VX_STRINGERIZE(VX_TYPE_BOOL),sizeof(vx_bool)*2},
    {VX_STRINGERIZE(VX_TYPE_ENUM),sizeof(vx_enum)*2},
    /* struct objects */
    {VX_STRINGERIZE(VX_TYPE_COORDINATES2D),sizeof(vx_coordinates2d_t)*2},
    {VX_STRINGERIZE(VX_TYPE_COORDINATES3D),sizeof(vx_coordinates3d_t)*2},
    {VX_STRINGERIZE(VX_TYPE_RECTANGLE),sizeof(vx_rectangle_t)*2},
    {VX_STRINGERIZE(VX_TYPE_KEYPOINT),sizeof(vx_keypoint_t)*2},
    /* data objects */
    {VX_STRINGERIZE(VX_TYPE_ARRAY),0},
    {VX_STRINGERIZE(VX_TYPE_DISTRIBUTION),0},
    {VX_STRINGERIZE(VX_TYPE_LUT),0},
    {VX_STRINGERIZE(VX_TYPE_IMAGE),0},
    {VX_STRINGERIZE(VX_TYPE_CONVOLUTION),0},
    {VX_STRINGERIZE(VX_TYPE_THRESHOLD),0},
    {VX_STRINGERIZE(VX_TYPE_MATRIX),0},
    {VX_STRINGERIZE(VX_TYPE_SCALAR),0},
    {VX_STRINGERIZE(VX_TYPE_PYRAMID),0},
    {VX_STRINGERIZE(VX_TYPE_REMAP),0},
#ifdef OPENVX_KHR_XML
    {VX_STRINGERIZE(VX_TYPE_IMPORT),0},
#endif
};

vx_int32 vxStringFromType(vx_enum type)
{
    vx_uint32 i = 0u;
    for (i = 0u; i < dimof(type_pairs); i++)
    {
        if (type == type_pairs[i].type)
        {
            return i;
        }
    }
    return -1;
}

#endif

#if defined(EXPERIMENTAL_USE_XML)

vx_status vxTypeFromString(char *string, vx_enum *type)
{
    vx_status status = VX_ERROR_INVALID_TYPE;
    vx_uint32 i = 0u;

    for (i = 0u; i < dimof(type_pairs); i++)
    {
        if (strncmp(string, type_pairs[i].name, sizeof(type_pairs[i].name)) == 0)
        {
            *type = type_pairs[i].type;
            status = VX_SUCCESS;
            break;
        }
    }
    return status;
}

vx_size vxMetaSizeOfType(vx_enum type)
{
    vx_size size = 0ul;
    vx_uint32 i = 0u;
    for (i = 0u; i < dimof(type_pairs); i++)
    {
        if (type_pairs[i].type == type)
        {
            size = type_pairs[i].nibbles / 2;
            break;
        }
    }
    return size;
}

#endif
