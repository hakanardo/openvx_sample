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

#ifndef _OPENVX_INT_TARGET_H_
#define _OPENVX_INT_TARGET_H_

/*!
 * \file
 * \brief The internal target implementation.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \defgroup group_int_target Internal Target API
 * \ingroup group_internal
 * \brief The Internal Target API.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Prints Target Information for Debugging.
 * \ingroup group_int_target
 */
void vxPrintTarget(vx_target_t *target, vx_uint32 index);

/*! \brief This allows the implementation to load a target interface into OpenVX.
 * \param [in] context The overall context pointer.
 * \param [in] name The shortened name of the target module.
 * \ingroup group_int_target
 */
vx_status vxLoadTarget(vx_context_t *context, vx_char *name);

/*! \brief This unloads a specific target in the targets list.
 * \param [in] context The overall context pointer.
 * \param [in] index The index into the context's target array.
 * \ingroup group_int_target
 */
vx_status vxUnloadTarget(vx_context_t *context, vx_uint32 index, vx_bool unload_module);

/*! \brief Initializes a target's kernels list.
 * \param [in] target The pointer to the target struct.
 * \param [in] kernels The array of kernels that the target supports.
 * \param [in] numkernels The length of the kernels list.
 * \ingroup group_int_target
 */
vx_status vxInitializeTarget(vx_target_t *target, vx_kernel_description_t *kernels[], vx_uint32 numkernels);

/*! \brief Deinitializes a target's kernels list.
 * \param [in] target The pointer to the target struct.
 * \param [in] kernels The array of kernels that the target supports.
 * \param [in] numkernels The length of the kernels list.
 * \ingroup group_int_target
 */
vx_status vxDeinitializeTarget(vx_target_t *target);

#ifdef __cplusplus
}
#endif

#endif
