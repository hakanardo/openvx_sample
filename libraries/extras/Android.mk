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
LOCAL_SRC_FILES := vx_extras_module.c \
	vx_filter.c \
	vx_gradients.c \
	vx_nonmax.c \
	vx_harris_score.c \
	vx_listers.c 
LOCAL_C_INCLUDES := $(OPENVX_INC)
LOCAL_SHARED_LIBRARIES := libdl libutils libcutils libbinder libhardware libion libgui libui libopenvx
LOCAL_MODULE := libopenvx-extras
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(OPENVX_DEFS)
LOCAL_SRC_FILES := vx_extras_lib.c
LOCAL_C_INCLUDES := $(OPENVX_INC)
LOCAL_STATIC_LIBRARIES := libopenvx-helper
LOCAL_MODULE := libopenvx-extras-lib
include $(BUILD_STATIC_LIBRARY)

