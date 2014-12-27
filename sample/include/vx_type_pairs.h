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

#ifndef _OPENVX_INT_type_pairs_H_
#define _OPENVX_INT_type_pairs_H_

/*! \brief The enum string structure
 * \ingroup group_int_type_pairs
 */
typedef struct _vx_enum_string_t {
    /*! \brief The data type enumeration */
    vx_enum type;
    /*! \brief A character string to hold the name of the data type enum */
    vx_char name[64];
    /*! \brief Value of how many nibbles the data type uses */
    uintmax_t nibbles;
} vx_enum_string_t;

extern vx_enum_string_t type_pairs[];

#ifdef __cplusplus
extern "C" {
#endif

vx_int32 vxStringFromType(vx_enum type);

#if defined (EXPERIMENTAL_USE_XML)
vx_status vxTypeFromString(char *string, vx_enum *type);
vx_size vxMetaSizeOfType(vx_enum type);
#endif

#ifdef __cplusplus
}
#endif

#endif
