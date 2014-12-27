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

#ifndef _OPENVX_INT_IMPORT_H_
#define _OPENVX_INT_IMPORT_H_

/*!
 * \file
 * \brief The Import Object Internal API.
 * \author Jesse Villarreal <jesse.villarreal@ti.com>
 *
 * \defgroup group_int_import Internal Import Object API
 * \ingroup group_internal
 * \brief The Internal Import Object API
 */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(EXPERIMENTAL_USE_XML)

/*! \brief Create an import object.
 * \param [in] context The context.
 * \param [in] type The type of import.
 * \param [in] count The number of references to import.
 * \ingroup group_int_import
 */
vx_import vxCreateImportInt(vx_context context, vx_enum type, vx_uint32 count);

/*! \brief Destroys an Import and it's scoped-objects.
 *  \param [in] ref The import reference object.
 *  \ingroup group_int_import
 */
void vxDestructImport(vx_reference ref);

#endif

#ifdef __cplusplus
}
#endif

#endif
