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

#ifndef _OPENVX_INT_KERNEL_H_
#define _OPENVX_INT_KERNEL_H_

/*!
 * \file
 * \brief The internal kernel implementation.
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_int_kernel Internal Kernel API
 * \ingroup group_internal
 * \brief The internal Kernel API.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Used to allocate a kernel object in the context.
 * \param [in] context The pointer to the context object.
 * \param [in] kenum The kernel enumeration value.
 * \param [in] function The pointer to the function of the kernel.
 * \param [in] name The name of the kernel in dotted notation.
 * \param [in] parameters The list of parameters for each kernel.
 * \param [in] numParams The number of parameters in the list.
 * \ingroup group_int_kernel
 */
vx_kernel_t *vxAllocateKernel(vx_context context,
                              vx_enum kenum,
                              vx_kernel_f function,
                              vx_char name[VX_MAX_KERNEL_NAME],
                              vx_param_description_t *parameters,
                              vx_uint32 numParams);

/*! \brief Used to initialize a kernel object in a target kernel list.
 * \param [in] context The pointer to the context object.
 * \param [in] kernel The pointer to the kernel structure.
 * \param [in] kenum The kernel enumeration value.
 * \param [in] function The pointer to the function of the kernel.
 * \param [in] name The name of the kernel in dotted notation.
 * \param [in] parameters The list of parameters for each kernel.
 * \param [in] numParams The number of parameters in the list.
 * \param [in] in_validator The function pointer to the input validator.
 * \param [in] out_validator The function pointer to the output validator.
 * \param [in] initialize The function to call to initialize the kernel.
 * \param [in] deinitialize The function to call to deinitialize the kernel.
 * \ingroup group_int_kernel
 */
vx_status vxInitializeKernel(vx_context context,
                             vx_kernel kernel,
                             vx_enum kenum,
                             vx_kernel_f function,
                             vx_char name[VX_MAX_KERNEL_NAME],
                             vx_param_description_t *parameters,
                             vx_uint32 numParams,
                             vx_kernel_input_validate_f in_validator,
                             vx_kernel_output_validate_f out_validator,
                             vx_kernel_initialize_f initialize,
                             vx_kernel_deinitialize_f deinitialize);

/*! \brief Used to deinitialize a kernel object in a target kernel list.
 * \param [in] kernel The pointer to the kernel structure.
 * \ingroup group_int_kernel
 */
vx_status vxDeinitializeKernel(vx_kernel *kernel);

/*! \brief Determines if a kernel is unique in the system.
 * \param kernel The handle to the kernel.
 * \ingroup group_int_kernel
 */
vx_bool vxIsKernelUnique(vx_kernel kernel);

#ifdef __cplusplus
}
#endif

#endif
