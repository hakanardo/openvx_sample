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
LOCAL_SRC_FILES := vx_xyz_module.c
LOCAL_C_INCLUDES := $(OPENVX_INC) $(LOCAL_PATH)
#LOCAL_SHARED_LIBRARIES := libdl libutils libcutils libbinder libhardware libion libgui libui
LOCAL_SHARED_LIBRARIES += libopenvx
LOCAL_MODULE := libxyz
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(OPENVX_DEFS)
LOCAL_SRC_FILES := vx_xyz_lib.c
LOCAL_C_INCLUDES := $(OPENVX_INC) $(LOCAL_PATH)
LOCAL_MODULE := libvx_xyz_lib
include $(BUILD_STATIC_LIBRARY)

ifeq ($(BUILD_EXAMPLE),1)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := $(OPENVX_DEFS)
LOCAL_SRC_FILES := vx_main.c vx_ar.c vx_corners.c vx_pipeline.c
LOCAL_C_INCLUDES := $(OPENVX_INC) $(LOCAL_PATH) $(OPENVX_TOP)/$(OPENVX_SRC)/extensions/include
LOCAL_STATIC_LIBRARIES := libvx_xyz_lib libopenvx-debug-lib
LOCAL_SHARED_LIBRARIES := libdl libutils libcutils libbinder libhardware libion libgui libui libopenvx
LOCAL_MODULE := vx_example
include $(BUILD_EXECUTABLE)
endif

