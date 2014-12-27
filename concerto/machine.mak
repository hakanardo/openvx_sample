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

define MACHINE_variables
ifneq ($(filter $($(1)_CPU),x86 X86 i386 i486 i586 i686),)
    HOST_PLATFORM=PC
    $(1)_FAMILY=X86
    $(1)_ARCH=32
    $(1)_ENDIAN=LITTLE
else ifneq ($(filter $($(1)_CPU),Intel64 amd64 X64),)
    HOST_PLATFORM=PC
    $(1)_FAMILY=X64
    $(1)_ARCH=64
    $(1)_ENDIAN=LITTLE
else ifeq ($($(1)__CPU),Power Macintosh)
    HOST_PLATFORM=PC
    $(1)_FAMILY=PPC
    $(1)_ARCH=32
    $(1)_ENDIAN=LITTLE
else ifeq ($($(1)_CPU),x86_64)
    HOST_PLATFORM=PC
    $(1)_FAMILY=x86_64
    $(1)_ARCH=64
    $(1)_ENDIAN=LITTLE
else ifneq ($(filter $($(1)_CPU),ARM M3 M4 A8 A8F A9 A9F A15 A15F armv7l),)
    ifeq ($(HOST_CPU),$($(1)_CPU))
        HOST_PLATFORM=PANDA
    else
        HOST_PLATFORM=PC
    endif
    $(1)_FAMILY=ARM
    $(1)_ARCH=32
    $(1)_ENDIAN=LITTLE
else ifneq ($(filter $($(1)_CPU),ARM64 aarch64 A53 A54 A57),)
    $(1)_FAMILY=ARM
    $(1)_ARCH=64
    $(1)_ENDIAN=LITTLE
else ifneq ($(filter $($(1)_CPU),C6XSIM C64T C64P C64 C66 C674 C67 C67P),)
    HOST_PLATFORM=PC
    $(1)_FAMILY=DSP
    $(1)_ARCH=32
    $(1)_ENDIAN=LITTLE
else ifeq ($($(1)_CPU),EVE)
    HOST_PLATFORM=PC
    $(1)_FAMILY=EVE
    $(1)_ARCH=32
    $(1)_ENDIAN=LITTLE
endif
endef


ifeq ($(HOST_OS),Windows_NT)
    $(info Windows Processor Architecture $(PROCESSOR_ARCHITECTURE))
    $(info Windows Processor Identification $(word 1, $(PROCESSOR_IDENTIFIER)))
    HOST_CPU=$(word 1, $(PROCESSOR_IDENTIFIER))
else
    HOST_CPU=$(shell uname -m)
endif

$(eval $(call MACHINE_variables,HOST))

