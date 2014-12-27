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

/*!
 * \file
 * \brief The Extra Extensions Module
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#include <VX/vx.h>
#include <VX/vx_lib_extras.h>
#include <VX/vx_helper.h>

#include <vx_extras_module.h>

/*! \brief Declares the list of all supported base kernels.
 * \ingroup group_debug_ext
 */
static vx_kernel_description_t *kernels[] = {
    &edge_trace_kernel,
    &euclidian_nonmax_kernel,
    &harris_score_kernel,
    &laplacian3x3_kernel,
    &lister_kernel,
    &nonmax_kernel,
    &norm_kernel,
    &scharr3x3_kernel,
    &sobelMxN_kernel,
};

/*! \brief Declares the number of base supported kernels.
 * \ingroup group_implementation
 */
static vx_uint32 num_kernels = dimof(kernels);

//**********************************************************************
//  PUBLIC FUNCTION
//**********************************************************************

/*! \brief The entry point into this module to add the base kernels to OpenVX.
 * \param context The handle to the implementation context.
 * \return vx_status Returns errors if some or all kernels were not added
 * correctly.
 * \ingroup group_implementation
 */
/*VX_API_ENTRY*/ vx_status VX_API_CALL vxPublishKernels(vx_context context)
{
    vx_status status = VX_FAILURE;
    vx_uint32 p = 0, k = 0;
    for (k = 0; k < num_kernels; k++)
    {
        vx_kernel kernel = vxAddKernel(context,
                             kernels[k]->name,
                             kernels[k]->enumeration,
                             kernels[k]->function,
                             kernels[k]->numParams,
                             kernels[k]->input_validate,
                             kernels[k]->output_validate,
                             kernels[k]->initialize,
                             kernels[k]->deinitialize);
        if (kernel)
        {
            status = VX_SUCCESS; // temporary
            for (p = 0; p < kernels[k]->numParams; p++)
            {
                status = vxAddParameterToKernel(kernel, p,
                                                kernels[k]->parameters[p].direction,
                                                kernels[k]->parameters[p].data_type,
                                                kernels[k]->parameters[p].state);
                if (status != VX_SUCCESS)
                {
                    vxAddLogEntry((vx_reference)context, status, "Failed to add parameter %d to kernel %s! (%d)\n", p, kernels[k]->name, status);
                    break;
                }
            }
            if (status == VX_SUCCESS)
            {
                status = vxFinalizeKernel(kernel);
                if (status != VX_SUCCESS)
                {
                    vxAddLogEntry((vx_reference)context, status, "Failed to finalize kernel[%u]=%s\n",k, kernels[k]->name);
                }
            }
            else
            {
                status = vxRemoveKernel(kernel);
                if (status != VX_SUCCESS)
                {
                    vxAddLogEntry((vx_reference)context, status, "Failed to remove kernel[%u]=%s\n",k, kernels[k]->name);
                }
            }
        }
        else
        {
            vxAddLogEntry((vx_reference)context, status, "Failed to add kernel %s\n", kernels[k]->name);
        }
    }
    return status;
}

