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

#ifndef _OPENVX_INT_DELAY_H_
#define _OPENVX_INT_DELAY_H_

/*!
 * \file
 * \brief The Delay Object Internal API.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \author Amit Shoham <amit@bdti.com>
 *
 * \defgroup group_int_delay Internal Delay Object API
 * \ingroup group_internal
 * \brief The Internal Delay Object API
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Frees the delay object memory.
 * \param [in] delay The delay object.
 * \ingroup group_int_delay
 */
void vxFreeDelay(vx_delay delay);

/*! \brief Allocates a delay object's meta-data memory.
 * \param [in] count The number of objects in the delay.
 * \ingroup group_int_delay
 */
vx_delay vxAllocateDelay(vx_size count);

/*! \brief Gets a reference from delay object.
 * \param [in] delay The delay object.
 * \param [in] index The relative index desired.
 * \note Relative indexes are typically negative in delay objects to indicate
 * the historical nature.
 * \ingroup group_int_delay
 */
vx_reference vxGetRefFromDelay(vx_delay  delay, vx_int32 index);

/*! \brief Adds an association to a node to a delay slot object reference.
 * \param [in] value The delay slot object reference.
 * \param [in] n The node reference.
 * \param [in] i The index of the parameter.
 * \param [in] d The direction of the parameter.
 */
vx_bool vxAddAssociationToDelay(vx_reference value,
                                vx_node n, vx_uint32 i);

/*! \brief Removes an association to a node from a delay slot object reference.
 * \param [in] value The delay slot object reference.
 * \param [in] n The node reference.
 * \param [in] i The index of the parameter.
 */
vx_bool vxRemoveAssociationToDelay(vx_reference value,
                                   vx_node n, vx_uint32 i);

/*! \brief Destroys a Delay and it's scoped-objects. */
void vxDestructDelay(vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif
