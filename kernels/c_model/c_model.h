/*
 * Copyright (c) 2011-2014 The Khronos Group Inc.
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

#ifndef _VX_C_MODEL_H_
#define _VX_C_MODEL_H_

#include <VX/vx.h>
#include <VX/vx_helper.h>
#include <math.h>

/*! \brief The largest convolution matrix the specification requires support for is 15x15.
 */
#define C_MAX_CONVOLUTION_DIM (15)

#ifdef __cplusplus
extern "C" {
#endif

vx_status vxAbsDiff(vx_image in1, vx_image in2, vx_image output);

vx_status vxAccumulate(vx_image input, vx_image accum);
vx_status vxAccumulateWeighted(vx_image input, vx_scalar scalar, vx_image accum);
vx_status vxAccumulateSquare(vx_image input, vx_scalar scalar, vx_image accum);

vx_status vxAddition(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output);
vx_status vxSubtraction(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output);

vx_status vxAnd(vx_image in0, vx_image in1, vx_image output);
vx_status vxOr(vx_image in0, vx_image in1, vx_image output);
vx_status vxXor(vx_image in0, vx_image in1, vx_image output);
vx_status vxNot(vx_image input, vx_image output);

vx_status vxChannelCombine(vx_image inputs[4], vx_image output);
vx_status vxChannelExtract(vx_image src, vx_scalar channel, vx_image dst);

vx_status vxConvertColor(vx_image src, vx_image dst);
vx_status vxConvertDepth(vx_image input, vx_image output, vx_scalar spol, vx_scalar sshf);

vx_status vxConvolve(vx_image src, vx_convolution conv, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxConvolution3x3(vx_image src, vx_image dst, vx_int16 conv[3][3], const vx_border_mode_t *borders);

vx_status vxFast9Corners(vx_image src, vx_scalar sens, vx_scalar nonm,
                         vx_array points, vx_scalar num_corners, vx_border_mode_t *bordermode);

vx_status vxMedian3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxBox3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxGaussian3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);

vx_status vxHistogram(vx_image src, vx_distribution dist);
vx_status vxEqualizeHist(vx_image src, vx_image dst);

vx_status vxIntegralImage(vx_image src, vx_image dst);
vx_status vxTableLookup(vx_image src, vx_lut lut, vx_image dst);

vx_status vxMeanStdDev(vx_image input, vx_scalar mean, vx_scalar stddev);
vx_status vxMinMaxLoc(vx_image input, vx_scalar minVal, vx_scalar maxVal, vx_array minLoc, vx_array maxLoc, vx_scalar minCount, vx_scalar maxCount);

vx_status vxErode3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxDilate3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);

vx_status vxMagnitude(vx_image grad_x, vx_image grad_y, vx_image output);

vx_status vxMultiply(vx_image in0, vx_image in1, vx_scalar scale_param, vx_scalar opolicy_param, vx_scalar rpolicy_param, vx_image output);

vx_status vxOpticalFlowPyrLK(/*p1, p2, p3...*/);

vx_status vxPhase(vx_image grad_x, vx_image grad_y, vx_image output);

vx_status vxScaleImage(vx_image src_image, vx_image dst_image, vx_scalar stype, vx_border_mode_t *bordermode, vx_float64 *interm, vx_size size);

vx_status vxSobel3x3(vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode);

vx_status vxThreshold(vx_image src_image, vx_threshold threshold, vx_image dst_image);

vx_status vxWarpPerspective(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders);
vx_status vxWarpAffine(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders);

#ifdef __cplusplus
}
#endif

#endif
