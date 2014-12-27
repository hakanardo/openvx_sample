# Copyright (c) 2012-2014 The Khronos Group Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and/or associated documentation files (the
# "Materials"), to deal in the Materials without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Materials, and to
# permit persons to whom the Materials are furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Materials.
#
# THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(OPENVX_DEFS)
LOCAL_SRC_FILES := \
	vx_context.c \
	vx_convolution.c \
	vx_delay.c \
	vx_distribution.c \
	vx_error.c \
	vx_graph.c \
	vx_image.c \
	vx_kernel.c \
	vx_list.c \
	vx_log.c \
	vx_lut.c \
	vx_matrix.c \
	vx_memory.c \
	vx_node_api.c \
	vx_node.c \
	vx_osal.c \
	vx_parameter.c \
	vx_pyramid.c \
	vx_reference.c \
	vx_remap.c \
	vx_scalar.c \
	vx_target.c \
	vx_threshold.c
LOCAL_C_INCLUDES := $(OPENVX_INC) $(OPENVX_TOP)/$(OPENVX_SRC)/include $(OPENVX_TOP)/debug
LOCAL_WHOLE_STATIC_LIBRARIES := libopenvx-helper libopenvx-debug
LOCAL_SHARED_LIBRARIES := libdl libutils libcutils libbinder libhardware libion libgui libui
LOCAL_MODULE := libopenvx
include $(BUILD_SHARED_LIBRARY)

