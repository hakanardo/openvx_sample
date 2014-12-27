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

$(info #######################################################################)
$(info TARGET_COMBO=$(TARGET_COMBO))
TARGET_COMBO_WORDS := $(subst :,$(SPACE),$(TARGET_COMBO))
TARGET_COMBO_COUNT := $(words $(TARGET_COMBO_WORDS))
ifeq ($(SHOW_MAKEDEBUG),1)
$(info TARGET_COMBO_WORDS=$(TARGET_COMBO_WORDS))
endif

ifeq ($(TARGET_COMBO_COUNT),6)
TARGET_PLATFORM := $(word 1,$(TARGET_COMBO_WORDS))
TARGET_OS       := $(word 2,$(TARGET_COMBO_WORDS))
TARGET_CPU      := $(word 3,$(TARGET_COMBO_WORDS))
TARGET_NUM_CORES:= $(word 4,$(TARGET_COMBO_WORDS))
TARGET_BUILD    := $(word 5,$(TARGET_COMBO_WORDS))
HOST_COMPILER   := $(word 6,$(TARGET_COMBO_WORDS))
else ifeq ($(TARGET_COMBO_COUNT),2)
TARGET_PLATFORM := $(word 1,$(TARGET_COMBO_WORDS))
TARGET_BUILD    := $(word 2,$(TARGET_COMBO_WORDS))
else
$(error Invalid COMBO syntax!)
endif
TARGET_OUT      := $(call MAKE_OUT,$(HOST_ROOT))
TARGET_DOC      := $(call MAKE_OUT,$(HOST_ROOT))/docs

TCWC := $(words $(subst =,$(SPACE),$(HOST_COMPILER)))
ifneq ($(TCWC),1)
TARGET_CROSS_COMPILE := $(word 2,$(subst =, ,$(HOST_COMPILER)))
$(if $(TARGET_CROSS_COMPILE),$(eval CROSS_COMPILE:=$(TARGET_CROSS_COMPILE)))
$(info CROSS_COMPILE=$(CROSS_COMPILE))
HOST_COMPILER := $(firstword $(subst =,$(SPACE),$(HOST_COMPILER)))
endif

# If the user is building for a remote core, they should set this variable
ifeq ($(HOST_PLATFORM),$(TARGET_PLATFORM))
# If the user supplied zero as the number of cores, we try to auto-detect on HOST. 
ifeq ($(TARGET_OS),Windows_NT)
$(if $(filter 0,$(TARGET_NUM_CORES)),$(eval TARGET_NUM_CORES=$(NUMBER_OF_PROCESSORS)))
else
$(if $(filter 0,$(TARGET_NUM_CORES)),$(eval TARGET_NUM_CORES=$(HOST_NUM_CORES)))
endif
else
# if they didn't set it to one.
$(if $(filter 0,$(TARGET_NUM_CORES)),$(eval TARGET_NUM_CORES=1))
endif

$(eval $(call MACHINE_variables,TARGET))

# name this with a module or dynamically renamed value...
TARGET_COMBO_NAME := $(subst :,_,$(TARGET_COMBO))
_NUM_PLATFORM_NAMES := $(words $(subst -, ,$(TARGET_PLATFORM)))
ifneq ($(_NUM_PLATFORM_NAMES),1)
_TARGET_PLATFORM := $(call uppercase,$(firstword $(subst -, ,$(TARGET_PLATFORM))))
else
_TARGET_PLATFORM := $(call uppercase,$(TARGET_PLATFORM))
endif

$(TARGET_COMBO_NAME)_DEFS := $(TARGET_OS) \
	$(TARGET_PLATFORM) \
	$(TARGET_CPU)="$(TARGET_CPU)" \
	TARGET_NUM_CORES=$(TARGET_NUM_CORES) TARGET_ARCH=$(TARGET_ARCH) \
	ARCH_$(TARGET_ARCH)
ifneq ($(TARGET_FAMILY),$(TARGET_CPU))
$(TARGET_COMBO_NAME)_DEFS += $(TARGET_FAMILY)
endif

ifeq ($(TARGET_BUILD),production)
$(TARGET_COMBO_NAME)_DEFS += TARGET_BUILD=0
else ifeq ($(TARGET_BUILD),release)
$(TARGET_COMBO_NAME)_DEFS += TARGET_BUILD=1
else ifeq ($(TARGET_BUILD),debug)
$(TARGET_COMBO_NAME)_DEFS += TARGET_BUILD=2
endif

# Include platform definition
include $(BUILD_PLATFORM)

# QNX Builds
ifdef QCONF_OVERRIDE
    include $(QCONF_OVERRIDE)
endif

# Include target definition
include $(BUILD_TARGET)
# Some of the target definitions may override the COMBO settings

$(TARGET_COMBO_NAME)_LDIRS := $(foreach proj,$(DEP_PROJECTS),$(call MAKE_OUT,$(proj)))

ifneq ($(NO_BANNER),1)
$(info HOST_ROOT=$(HOST_ROOT))
$(info HOST_COMPILER=$(HOST_COMPILER))
$(info HOST_PLATFORM=$(HOST_PLATFORM))
$(info HOST_OS=$(HOST_OS))
$(info HOST_CPU=$(HOST_CPU))
$(info HOST_ARCH=$(HOST_ARCH))
$(info HOST_FAMILY=$(HOST_FAMILY))
$(info HOST_NUM_CORES=$(HOST_NUM_CORES))
$(info TARGET_OS=$(TARGET_OS))
$(info TARGET_CPU=$(TARGET_CPU))
$(info TARGET_ARCH=$(TARGET_ARCH))
$(info TARGET_FAMILY=$(TARGET_FAMILY))
$(info TARGET_NUM_CORES=$(TARGET_NUM_CORES))
$(info TARGET_PLATFORM=$(TARGET_PLATFORM))
$(info TARGET_BUILD=$(TARGET_BUILD))
$(info TARGET_OUT=$(TARGET_OUT))
$(info TARGET_DOC=$(TARGET_DOC))
$(info $(TARGET_COMBO_NAME)_DEFS = $($(TARGET_COMBO_NAME)_DEFS))
$(info $(TARGET_COMBO_NAME)_LDIRS = $($(TARGET_COMBO_NAME)_LDIRS))
ifeq ($(SHOW_MAKEDEBUG),1)
$(info TARGET_MAKEFILES=$(TARGET_MAKEFILES))
endif
$(info SCM_ROOT=$(SCM_ROOT))
$(info SCM_VERSION=$(SCM_VERSION))
endif

include $(TARGET_MAKEFILES)

# Define a rule to take care of libraries that are actual "system" libraries and 
# are not present in the build, they will look like TDIR libraries.
$(addprefix $(TARGET_OUT)/%,.so .dll .a .lib)::
	@echo Required system library $(notdir $@)

