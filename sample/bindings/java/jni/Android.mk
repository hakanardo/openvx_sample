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

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_CPPFLAGS := $(OPENVX_DEFS)
LOCAL_SRC_FILES := openvx_jni.cpp \
                   Context.cpp \
                   Image.cpp \
                   Buffer.cpp \
                   Scalar.cpp \
                   Kernel.cpp \
                   Graph.cpp \
                   Node.cpp \
                   Parameter.cpp \
                   Reference.cpp \
                   Target.cpp
                   # Convolution.cpp \
                   # Distribution.cpp \
                   # Matrix.cpp \

LOCAL_C_INCLUDES += $(JNI_H_INCLUDE) $(OPENVX_INC) $(OPENVX_TOP)/debug
LOCAL_SHARED_LIBRARIES += \
    libopenvx \
    libvxu \
    libcutils \
    libhardware \
    libhardware_legacy \
    libnativehelper \
    libsystem_server \
    libutils \
    libui \
    libdl \
    libbinder \
    libgui \
    libmedia

ifeq ($(TARGET_SIMULATOR),true)
ifeq ($(TARGET_OS),linux)
ifeq ($(TARGET_ARCH),x86)
LOCAL_LDLIBS += -lpthread -ldl -lrt
endif
endif
endif
ifeq ($(WITH_MALLOC_LEAK_CHECK),true)
    LOCAL_CFLAGS += -DMALLOC_LEAK_CHECK
endif
LOCAL_MODULE:= libopenvx_jni
include $(BUILD_SHARED_LIBRARY)
