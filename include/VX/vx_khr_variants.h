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

#ifndef _VX_KHR_VARIANT_H_
#define _VX_KHR_VARIANT_H_

/*!
 * \file
 * \brief The Khronos Extension for Kernel Variants.
 * \author Erik Rainey <erik.rainey@gmail.com>
 * \author Erez Natan <erez.natan@samsung.com>
 *
 * \defgroup group_variants Extension: Kernel Variants
 * \brief The Khronos Extension for Kernel Variants.
 * \details Kernel Variants allow the Client-Defined Functions to create several
 * kernels on the same target with the same name, but with slight variations
 * between them. Frequently these variants are expected to employ different
 * algorithms or methodologies.
 *
 * All target specific kernels and target variants must conform to the same OpenVX
 * specification of the OpenVX Kernel in order to use the string name and enumeration.
 * For example, a vendor may supply multiple targets,
 * and implement the same functionality on each. Futhermore the same
 * vendor may offer a variant on some specific target which offers some differentiation but
 * still  conforms to the definition of the OpenVX Kernel.
 * In this example there are 3 implementations of the same computer vision function, "Sobel3x3".
 * \arg On "CPU" a "Sobel3x3" which is "faster". A variant which may produce slightly less accurate but still conformant results.
 * \arg On "CPU" a "Sobel3x3" which is more "accurate". A variant which may run slower but produces bit exact results.
 * \arg On "GPU" a "Sobel3x3" \e default variant which may run on a remote core and produce bit exact results.
 *
 * In each of the cases a client of OpenVX could request the kernels in nearly
 * the same the same manner. There are two main approaches, which depend on the
 * method a client calls to get the kernel reference. The first uses enumerations.
 * \snippet examples/vx_kernels.c firstmethod
 * This method allows to client to attempt to find other targets and variants, but if
 * these are not present, the default node would still have been constructed.
 * The second method depends on using fully qualified strings to get the kernel reference.
 * \snippet examples/vx_kernels.c secondmethod
 * This second method is more compact but is does not permit fail-safing to default versions.
 *
 * As part of this extension, the function <tt>vxGetKernelByName</tt> will now accept more
 * qualifications to the string naming scheme. Kernels names can be additionally
 * qualified in 2 separate ways, by target and by variant. A "fully" qualified name is in the format of
 * <i>target</i><b>:</b><i>kernel</i><b>:</b><i>variant</i>.
 * Both \e target and \e variant may be omitted (for an unqualified name).
 * In this case, the implementation will assume the "default" value of these
 * names (which could literally be "default"). Names may also be fully
 * qualified with target included.
 * Examples:
 * \arg "khronos.c_model:org.khonos.openvx.sobel3x3:default" - fully qualified
 * \arg "org.khronos.openvx.sobel3x3:default" (missing target) - partially qualified
 * \arg "khronos.c_model:org.khronos.openvx.sobel3x3" (missing variant) - partially qualifed.
 * \arg "org.khronos.openvx.sobel3x3" - unqualified.
 *
 */

/*! \brief The string name of the extension.
 * \ingroup group_variants
 */
#define OPENVX_KHR_VARIANTS  "vx_khr_variants"

/*! \brief Defines the maximum number of characters in a variant string.
 * \ingroup group_variants
 */
#define VX_MAX_VARIANT_NAME (64)

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Used to choose a variant of a kernel for execution on a particular node.
 * \param [in] node The reference to the node.
 * \param [in] variantName The name of the variant to choose.
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \ingroup group_variants
 */
VX_API_ENTRY vx_status VX_API_CALL vxChooseKernelVariant(vx_node node, vx_char variantName[VX_MAX_VARIANT_NAME]);

#ifdef __cplusplus
}
#endif

#endif

