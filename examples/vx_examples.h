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

/*!
 * \file vx_examples.h
 * \brief A header of example functions.
 * \author Erik Rainey <erik.rainey@gmail.com>
 *
 * \defgroup group_example OpenVX Examples
 * \brief A series of examples of how to use the OpenVX API.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <VX/vx.h>
#include <VX/vx_lib_debug.h>
#include <VX/vx_helper.h>

int example_pipeline(int argc, char *argv[]);
int example_corners(int argc, char *argv[]);
int example_ar(int argc, char *argv[]);
int example_super_resolution(int argc, char *argv[]);
