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

#ifndef _OPENVX_INT_CONTEXT_H_
#define _OPENVX_INT_CONTEXT_H_

/*!
 * \file
 * \brief
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_context Internal Context API
 * \ingroup group_internal
 * \brief Internal Context API
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief The implementation string which is of the format "<vendor>.<substring>" */
extern const vx_char implementation[];

/*! \brief This returns true if the type is within the definition of types in OpenVX.
 * \note VX_TYPE_INVALID is not valid for determining a type.
 * \param [in] type The \ref vx_type_e value.
 * \ingroup group_int_context
 */
vx_bool vxIsValidType(vx_enum type);

/*! \brief This determines if the import type is supported.
 * \param [in] type The \ref vx_import_type_e value.
 * \ingroup group_int_context
 */
vx_bool vxIsValidImport(vx_enum type);

/*! \brief This determines if a context is valid.
 * \param [in] context The pointer to the context to test.
 * \retval vx_true_e The context is valid.
 * \retval vx_false_e The context is not valid.
 * \ingroup group_int_context
 */
vx_bool vxIsValidContext(vx_context context);

/*! \brief Searches the accessors list to find an open spot and then
 * will allocate memory if needed.
 * \ingroup group_int_context
 */
vx_bool vxAddAccessor(vx_context context,
                      vx_size size,
                      vx_enum usage,
                      void *ptr,
                      vx_reference ref,
                      vx_uint32 *pIndex);

/*! \brief Finds the accessor in the list and returns the index.
 * \ingroup group_int_context
 */
vx_bool vxFindAccessor(vx_context context, void *ptr, vx_uint32 *pIndex);

/*! \brief Finds and removes an accessor from the list.
 * \ingroup group_int_context
 */
void vxRemoveAccessor(vx_context context, vx_uint32 index);

#ifdef __cplusplus
}
#endif

#endif
