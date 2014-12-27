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

ifeq ($(TARGET_OS),LINUX)
    PLATFORM_LIBS := dl pthread rt
else ifeq ($(TARGET_OS),DARWIN)
    PLATFORM_LIBS :=
else ifeq ($(TARGET_OS),Windows_NT)
    PLATFORM_LIBS := Ws2_32 user32 kernel32 Gdi32
else ifeq ($(TARGET_OS),__QNX__)
    PLATFORM_LIBS := screen socket
else ifeq ($(TARGET_OS),CYGWIN)
    PLATFORM_LIBS := c pthread
endif

