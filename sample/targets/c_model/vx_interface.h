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

#ifndef _OPENVX_INTERFACE_H_
#define _OPENVX_INTERFACE_H_

#include <c_model.h>
#include <VX/vx_helper.h>

extern vx_kernel_description_t colorconvert_kernel;
extern vx_kernel_description_t channelextract_kernel;
extern vx_kernel_description_t sobel3x3_kernel;
extern vx_kernel_description_t magnitude_kernel;
extern vx_kernel_description_t phase_kernel;
extern vx_kernel_description_t channelcombine_kernel;
extern vx_kernel_description_t scale_image_kernel;
extern vx_kernel_description_t lut_kernel;
extern vx_kernel_description_t histogram_kernel;
extern vx_kernel_description_t equalize_hist_kernel;
extern vx_kernel_description_t absdiff_kernel;
extern vx_kernel_description_t mean_stddev_kernel;
extern vx_kernel_description_t threshold_kernel;
extern vx_kernel_description_t integral_image_kernel;
extern vx_kernel_description_t erode3x3_kernel;
extern vx_kernel_description_t dilate3x3_kernel;
extern vx_kernel_description_t median3x3_kernel;
extern vx_kernel_description_t box3x3_kernel;
extern vx_kernel_description_t box3x3_kernel_2;
extern vx_kernel_description_t gaussian3x3_kernel;
extern vx_kernel_description_t laplacian3x3_kernel;
extern vx_kernel_description_t convolution_kernel;
extern vx_kernel_description_t pyramid_kernel;
extern vx_kernel_description_t accumulate_kernel;
extern vx_kernel_description_t accumulate_weighted_kernel;
extern vx_kernel_description_t accumulate_square_kernel;
extern vx_kernel_description_t minmaxloc_kernel;
extern vx_kernel_description_t convertdepth_kernel;
extern vx_kernel_description_t canny_kernel;
extern vx_kernel_description_t scharr3x3_kernel;
extern vx_kernel_description_t and_kernel;
extern vx_kernel_description_t or_kernel;
extern vx_kernel_description_t xor_kernel;
extern vx_kernel_description_t not_kernel;
extern vx_kernel_description_t multiply_kernel;
extern vx_kernel_description_t add_kernel;
extern vx_kernel_description_t subtract_kernel;
extern vx_kernel_description_t warp_affine_kernel;
extern vx_kernel_description_t warp_perspective_kernel;
extern vx_kernel_description_t harris_kernel;
extern vx_kernel_description_t fast9_kernel;
extern vx_kernel_description_t optpyrlk_kernel;
extern vx_kernel_description_t remap_kernel;
extern vx_kernel_description_t halfscale_gaussian_kernel;

#endif

