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

SYSIDIRS := $(HOST_ROOT)/include
SYSLDIRS :=
SYSDEFS  := OPENVX_BUILDING OPENVX_USE_SMP
#SYSDEFS  += OPENVX_USE_TILING
#SYSDEFS  += EXPERIMENTAL_USE_TARGET
#SYSDEFS  += EXPERIMENTAL_USE_VARIANTS
#SYSDEFS  += EXPERIMENTAL_USE_NODE_MEMORY

ifeq ($(TARGET_BUILD),debug)
SYSDEFS += OPENVX_DEBUGGING
endif

ifeq ($(TARGET_PLATFORM),PC)
    ifneq ($(OPENCL_ROOT),) 
        SYSDEFS += EXPERIMENTAL_USE_OPENCL CL_USE_LUMINANCE
        USE_OPENCL := true
    endif
    
    ifeq ($(TARGET_OS),LINUX)
        INSTALL_LIB := /usr/lib
        INSTALL_BIN := /usr/bin
        INSTALL_INC := /usr/include
        SYSIDIRS += /usr/include
        SYSLDIRS += /usr/lib
        SYSDEFS += _XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1
        SYSDEFS += EXPERIMENTAL_USE_DOT EXPERIMENTAL_USE_OPENMP        # should be "libxml-2.0" on Ubuntu
        ifneq ($(XML2_PKG),)
            XML2_LIBS := $(subst -l,,$(shell pkg-config --libs-only-l $(XML2_PKG)))
            XML2_INCS := $(subst -I,,$(shell pkg-config --cflags $(XML2_PKG)))
            SYSDEFS += EXPERIMENTAL_USE_XML
            SYSIDIRS += $(XML2_INCS)
        endif
        ifneq ($(SDL_PKG),)
            SDL_LIBS := $(subst -l,,$(shell pkg-config --libs-only-l $(SDL_PKG)))
            SDL_INCS := $(subst -I,,$(shell pkg-config --cflags-only-I $(SDL_PKG)))
            SYSDEFS += $(subst -D,,$(shell pkg-config --cflags-only-other $(SDL_PKG))) OPENVX_USE_SDL
            SYSIDIRS += $(SDL_INCS)
        endif
    else ifeq ($(TARGET_OS),DARWIN)
        INSTALL_LIB := /opt/local/lib
        INSTALL_BIN := /opt/local/bin
        INSTALL_INC := /opt/local/include
        SYSDEFS += _XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1
        SYSDEFS += EXPERIMENTAL_USE_DOT
        XML2_PATH := $(dir $(shell brew list libxml2 | grep -m 1 include))
        XML2_BREWROOT := $(patsubst %/include/libxml2/libxml/,%,$(XML2_PATH))
        ifneq ($(XML2_BREWROOT),)
            XML2_LIBS := xml2
            SYSDEFS += EXPERIMENTAL_USE_XML
            SYSIDIRS += $(XML2_BREWROOT)/include/libxml2
            SYSLDIRS += $(XML2_BREWROOT)/lib
        endif
    else ifeq ($(TARGET_OS),CYGWIN)
        INSTALL_LIB := /usr/lib
        INSTALL_BIN := /usr/bin
        INSTALL_INC := /usr/include
        SYSDEFS += _XOPEN_SOURCE=700 _BSD_SOURCE=1 _GNU_SOURCE=1 WINVER=0x501
        ifneq (,$(findstring OPENVX_BUILDING,$(SYSDEFS)))
            SYSDEFS += 'VX_API_ENTRY=__attribute__((dllexport))'
        else
            SYSDEFS += 'VX_API_ENTRY=__attribute__((dllimport))'
        endif
    else ifeq ($(TARGET_OS),Windows_NT)
        INSTALL_LIB := "${windir}\\system32"
        INSTALL_BIN := "${windir}\\system32"
        INSTALL_INC :=
        SYSIDIRS += $(HOST_ROOT)/include/windows
        SYSDEFS += WIN32_LEAN_AND_MEAN WIN32 _WIN32 _CRT_SECURE_NO_DEPRECATE WINVER=0x0501 _WIN32_WINNT=0x0501
        ifneq (,$(findstring OPENVX_BUILDING,$(SYSDEFS)))
            SYSDEFS += VX_API_ENTRY=__declspec(dllexport)
        else
            SYSDEFS += VX_API_ENTRY=__declspec(dllimport)
        endif
    endif
endif

