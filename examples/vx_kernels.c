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
#include <VX/vx_helper.h>
#if defined(EXPERIMENTAL_USE_VARIANTS)
#include <VX/vx_khr_variants.h>
#endif
#if defined(EXPERIMENTAL_USE_TARGET)
#include <VX/vx_ext_target.h>
#endif


vx_node vx_create_specific_sobel(vx_context context, vx_graph graph, vx_bool easy)
{
    vx_node n = 0;
    if (easy == vx_false_e) {
        //! [firstmethod]
        vx_kernel kernel = vxGetKernelByEnum(context, VX_KERNEL_SOBEL_3x3);
        vx_node node = vxCreateGenericNode(graph, kernel);
        //! [firstmethod]
#if defined(EXPERIMENTAL_USE_TARGET)
        vx_target target = vxGetTargetByName(context, "gpu");
        vxAssignNodeAffinity(node, target);
#endif
#if defined(EXPERIMENTAL_USE_VARIANTS)
        //! [variant:firstmethod]
        vxChooseKernelVariant(node, "faster");
        //! [variant:firstmethod]
#endif
        n = node;
    } else {
#if defined(EXPERIMENTAL_USE_VARIANTS)
        //! [variant:second]
#if defined(EXPERIMENTAL_USE_TARGET)
        vx_kernel kernel = vxGetKernelByName(context, "cpu:org.khronos.openvx.sobel3x3:faster");
#else
        vx_kernel kernel = vxGetKernelByName(context, "org.khronos.openvx.sobel3x3:faster");
#endif
        vx_node node = vxCreateGenericNode(graph, kernel);
        //! [variant:second]
#else   /*defined(EXPERIMENTAL_USE_VARIANTS)*/
        //! [secondmethod]
        vx_kernel kernel = vxGetKernelByName(context, "org.khronos.openvx.sobel3x3");
        vx_node node = vxCreateGenericNode(graph, kernel);
        //! [secondmethod]
#endif
        n = node;
    }
    return n;
}

