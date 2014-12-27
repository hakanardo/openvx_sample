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

ifeq ($(filter $(HOST_COMPILER),GCC CLANG),$(HOST_COMPILER))
include $(PRELUDE)
ifneq (,$(findstring EXPERIMENTAL_USE_OPENMP,$(SYSDEFS)))
SKIPBUILD:=
USE_OPENMP:=true
else
SKIPBUILD:=1
endif
TARGET := openvx-openmp
TARGETTYPE := dsmo
DEFFILE := openvx-target.def
CSOURCES = $(call all-c-files)
IDIRS += $(HOST_ROOT)/$(OPENVX_SRC)/include $(HOST_ROOT)/debug
SHARED_LIBS := openvx vxu
STATIC_LIBS := openvx-debug-lib openvx-extras-lib openvx-helper openvx-c_model-lib
include $(FINALE)
endif
