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

#ifndef _OPENVX_INT_ERROR_H_
#define _OPENVX_INT_ERROR_H_

/*!
 * \file
 * \brief The internal error implementation
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_error Internal Error API
 * \ingroup group_internal
 * \brief The Internal Error API.
 */

#ifdef __cplusplus
    extern "C" {
#endif

/*! \brief Creates all the status codes as error objects.
 * \ingroup group_int_error
 */
vx_bool vxCreateConstErrors(vx_context_t *context);

/*! \brief Releases an error object.
 * \ingroup group_int_error
 */
void vxReleaseErrorInt(vx_error_t **error);

/*! \brief Creates an Error Object.
 * \ingroup group_int_error
 */
vx_error_t *vxAllocateError(vx_context_t *context, vx_status status);

/*! \brief Matches the status code against all known error objects in the
 * context.
 * \param [in] context The pointer to the overall context.
 * \param [in] status The status code to find.
 * \return Returns a matching error object.
 */
vx_error_t *vxGetErrorObject(vx_context_t *context, vx_status status);

#ifdef __cplusplus
}
#endif

#endif

