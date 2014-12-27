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
 * \brief The Extra Extensions Module Header.
 * \author Erik Rainey <erik.rainey@gmail.com>
 */

#ifndef _VX_EXTRAS_MODULE_H_
#define _VX_EXTRAS_MODULE_H_

#ifdef  __cplusplus
extern "C" {
#endif

extern vx_kernel_description_t edge_trace_kernel;
extern vx_kernel_description_t euclidian_nonmax_kernel;
extern vx_kernel_description_t harris_score_kernel;
extern vx_kernel_description_t laplacian3x3_kernel;
extern vx_kernel_description_t lister_kernel;
extern vx_kernel_description_t nonmax_kernel;
extern vx_kernel_description_t norm_kernel;
extern vx_kernel_description_t scharr3x3_kernel;
extern vx_kernel_description_t sobelMxN_kernel;

#ifdef  __cplusplus
}
#endif

#endif  /* _VX_EXTRAS_MODULE_H_ */

