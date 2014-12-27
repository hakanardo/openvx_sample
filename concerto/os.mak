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

ifeq ($(OS),Windows_NT)
    ifeq ($(TERM),cygwin)
        HOST_OS=CYGWIN
        HOST_NUM_CORES := $(shell cat /proc/cpuinfo | grep processor | wc -l)
    else ifeq ($(TERM),xterm)
        HOST_OS=CYGWIN
        P2W_CONV=$(patsubst \cygdrive\c\%,c:\%,$(subst /,\,$(1)))
        W2P_CONV=$(subst \,/,$(patsubst C:\%,\cygdrive\c\% $(1)))
        HOST_NUM_CORES := $(shell cat /proc/cpuinfo | grep processor | wc -l)
    else
        HOST_OS=Windows_NT
        CL_ROOT?=$(VCINSTALLDIR)
        HOST_NUM_CORES := $(NUM_PROCESSORS)
    endif
else
    OS=$(shell uname -s)
    ifeq ($(OS),Linux)
        HOST_OS=LINUX
        HOST_NUM_CORES := $(shell cat /proc/cpuinfo | grep processor | wc -l)
    else ifeq ($(OS),Darwin)
        HOST_OS=DARWIN
        HOST_NUM_CORES := $(word 2,$(shell sysctl hw.ncpu))
    else ifeq ($(OS),CYGWIN_NT-5.1)
        HOST_OS=CYGWIN
        P2W_CONV=$(patsubst \cygdrive\c\%,c:\%,$(subst /,\,$(1)))
        W2P_CONV=$(subst \,/,$(patsubst C:\%,\cygdrive\c\% $(1)))
        HOST_NUM_CORES := $(shell cat /proc/cpuinfo | grep processor | wc -l)
    else
        HOST_OS=POSIX
    endif
endif

# PATH_CONV and set HOST_COMPILER if not yet specified
ifeq ($(HOST_OS),Windows_NT)
    STRING_ESCAPE=$(subst \,\\,$(1))
    PATH_CONV=$(subst /,\,$(1))
    PATH_SEP=\\
    PATH_SEPD=$(strip \)
    HOST_COMPILER?=CL
else
    STRING_ESCAPE=$(1)
    PATH_CONV=$(1)
    PATH_SEP=/
    PATH_SEPD=/
    HOST_COMPILER?=GCC
endif

ifeq ($(HOST_OS),Windows_NT)
$(info ComSpec=$(ComSpec))
    SHELL:=$(ComSpec)
    .SHELLFLAGS=/C
else
    SHELL:=/bin/sh
endif

$(info SHELL=$(SHELL))
