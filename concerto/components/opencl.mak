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

# @author Erik Rainey
# @url http://github.com/emrainey/Concerto

ifeq ($(USE_OPENCL),true)
    OCL_LIB ?= OpenCL
    ifeq ($(HOST_OS),Windows_NT)
        ifeq ($(OPENCL_ROOT),)
            $(error OPENCL_ROOT must be defined to use OPENCL_ROOT)
        endif
        IDIRS += $(OPENCL_ROOT)/include $(OPENCL_ROOT)/inc
        LDIRS += $(OPENCL_ROOT)/lib $(OPENCL_ROOT)/lib64
        ifeq ($(filter $(PLATFORM_LIBS),$(OCL_LIB)),)
            PLATFORM_LIBS += $(OCL_LIB)
        endif
    else ifeq ($(HOST_OS),LINUX)
        # User should install GLUT/Mesa via package system
        ifneq ($(OPENCL_ROOT),)
            IDIRS += $(OPENCL_ROOT)/include $(OPENCL_ROOT)/inc
            LDIRS += $(OPENCL_ROOT)/lib $(OPENCL_ROOT)/lib64
        endif
        ifeq ($(filter $(PLATFORM_LIBS),$(OCL_LIB)),)
            PLATFORM_LIBS += $(OCL_LIB)
        endif
    else ifeq ($(HOST_OS),DARWIN)
        # User should have XCode install OpenCL
        $(_MODULE)_FRAMEWORKS += -framework OpenCL
    endif
    
    # OpenCL-Environment Defines
    ifeq ($(HOST_OS),CYGWIN)
        DEFS += KDIR="\"$(KDIR)\"" CL_USER_DEVICE_COUNT=$(CL_USER_DEVICE_COUNT) CL_USER_DEVICE_TYPE="\"$(CL_USER_DEVICE_TYPE)\""
    else ifeq ($(HOST_OS),Windows_NT)
        DEFS += KDIR="$(call PATH_CONV,$(KDIR))\\" CL_USER_DEVICE_COUNT=$(CL_USER_DEVICE_COUNT) CL_USER_DEVICE_TYPE="$(CL_USER_DEVICE_TYPE)"
    else
        DEFS += KDIR="$(KDIR)" CL_USER_DEVICE_COUNT=$(CL_USER_DEVICE_COUNT) CL_USER_DEVICE_TYPE="$(CL_USER_DEVICE_TYPE)" $(if $(CL_DEBUG),CL_DEBUG=1)
    endif
endif
