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

# Any OS can build itself and maybe some secondary OS's

ifeq ($(SHOW_MAKEDEBUG),1)
$(info Starting COMBOS:)
$(foreach combo,$(TARGET_COMBOS),$(info TARGET_COMBOS+=$(combo)))
endif

ifeq ($(HOST_PLATFORM),PC)
ifneq ($(HOST_CPU),ARM)
# PANDA ARM is self-hosting
TARGET_COMBOS := $(call FILTER_OUT_COMBO,PANDA)
endif
ifeq ($(HOST_FAMILY),X86)
TARGET_COMBOS := $(call FILTER_OUT_COMBO,x86_64)
endif
ifeq ($(HOST_OS),LINUX)
TARGET_COMBOS := $(call FILTER_COMBO,LINUX SYSBIOS __QNX__)
else ifneq ($(filter $(HOST_OS),CYGWIN DARWIN),)
TARGET_COMBOS := $(call FILTER_COMBO,$(HOST_OS))
else ifeq ($(HOST_OS),Windows_NT)
TARGET_COMBOS := $(call FILTER_COMBO,$(HOST_OS) SYSBIOS)
endif
else ifeq ($(HOST_PLATFORM),PANDA)
TARGET_COMBOS := $(call FILTER_COMBO,$(HOST_PLATFORM))
endif

# If the platform is set, remove others which are not on that platform.
ifneq ($(filter $(origin TARGET_PLATFORM),environment command),)
$(info Keep only $(TARGET_PLATFORM) platform in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_PLATFORM))
endif

# If the OS is set, remove others which are not on that OS.
ifneq ($(filter $(origin TARGET_OS),environment command),)
$(info Keep only $(TARGET_OS) OS in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_OS))
endif

# If the CPU is set, remove others which are not on that CPU.
ifneq ($(filter $(origin TARGET_CPU),environment command),)
$(info Keep only $(TARGET_CPU) CPU in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_CPU))
endif

# If the BUILD is set, remove others which are not on that BUILD
ifneq ($(filter $(origin TARGET_BUILD),environment command),)
$(info Keep only $(TARGET_BUILD) BUILDS in TARGET_COMBOS)
TARGET_COMBOS := $(call FILTER_COMBO,$(TARGET_BUILD))
endif

# The compilers which must have roots set. 
COMPILER_ROOTS := 
ifeq ($(HOST_OS),Windows_NT)
COMPILER_ROOTS += GCC_ROOT
endif

# The compiler which do not have roots set.
REMOVE_ROOTS := $(foreach root,$(COMPILER_ROOTS),$(if $(filter $(origin $(root)),undefined),$(root)))

# Remove the list of combos which can not be built
TARGET_COMBOS := $(call FILTER_OUT_COMBO,$(foreach root,$(REMOVE_ROOTS),$(subst _ROOT,,$(root))))

TARGET_COMBOS := $(strip $(TARGET_COMBOS))

ifeq ($(SHOW_MAKEDEBUG),1)
$(info Remaining COMBOS:)
$(foreach combo,$(TARGET_COMBOS),$(info TARGET_COMBOS+=$(combo)))
endif

$(if $(strip $(TARGET_COMBOS)),,$(error No TARGET_COMBOS remain! Nothing to make))
