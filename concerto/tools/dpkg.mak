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

CHECK_DPKG := $(shell which dpkg)
ifeq ($(SHOW_MAKEDEBUG),1)
$(info CHECK_DPKG=$(CHECK_DPKG))
endif
ifneq ($(CHECK_DPKG),)

PKG_EXT := .deb

VARS=$(shell "dpkg-architecture")
$(foreach var,$(VARS),$(if $(findstring DEB_BUILD_ARCH,$(var)),$(eval $(var))))

$(_MODULE)_CFG         ?= control
$(_MODULE)_PKG_NAME    := $(subst _,-,$($(_MODULE)_TARGET))
$(_MODULE)_PKG_FLDR    := $($(_MODULE)_TDIR)/$($(_MODULE)_PKG_NAME)
$(_MODULE)_PKG         := $($(_MODULE)_PKG_NAME)$(PKG_EXT)
$(_MODULE)_BIN         := $($(_MODULE)_TDIR)/$($(_MODULE)_PKG)

ifeq ($(SHOW_MAKEDEBUG),1)
$(info $(_MODULE)_PKG_FLDR=$($(_MODULE)_PKG_FLDR))
endif

# Remember that the INSTALL variable tend to be based in /
$(_MODULE)_PKG_LIB := $($(_MODULE)_PKG_FLDR)$($(_MODULE)_INSTALL_LIB)
$(_MODULE)_PKG_INC := $($(_MODULE)_PKG_FLDR)$($(_MODULE)_INSTALL_INC)/$($(_MODULE)_INC_SUBPATH)
$(_MODULE)_PKG_BIN := $($(_MODULE)_PKG_FLDR)$($(_MODULE)_INSTALL_BIN)
$(_MODULE)_PKG_CFG := $($(_MODULE)_PKG_FLDR)/DEBIAN

ifeq ($(SHOW_MAKEDEBUG),1)
$(info $(_MODULE)_PKG_LIB=$($(_MODULE)_PKG_LIB))
$(info $(_MODULE)_PKG_INC=$($(_MODULE)_PKG_INC))
$(info $(_MODULE)_PKG_BIN=$($(_MODULE)_PKG_BIN))
$(info $(_MODULE)_PKG_CFG=$($(_MODULE)_PKG_CFG))
endif

$(_MODULE)_PKG_DEPS:= $(foreach lib,$($(_MODULE)_SHARED_LIBS),$($(_MODULE)_PKG_LIB)/lib$(lib).so) \
                      $(foreach lib,$($(_MODULE)_STATIC_LIBS),$($(_MODULE)_PKG_LIB)/lib$(lib).a) \
                      $(foreach bin,$($(_MODULE)_BINS),$($(_MODULE)_PKG_BIN)/$(bin)) \
                      $(foreach inc,$($(_MODULE)_INCS),$($(_MODULE)_PKG_INC)/$(notdir $(inc)))

ifeq ($(SHOW_MAKEDEBUG),1)
$(info $(_MODULE)_PKG_DEPS=$($(_MODULE)_PKG_DEPS))
endif

$(_MODULE)_OBJS := $($(_MODULE)_PKG_CFG)/$($(_MODULE)_CFG) $($(_MODULE)_PKG_DEPS)

ifeq ($(SHOW_MAKEDEBUG),1)
$(info $(_MODULE)_OBJS=$($(_MODULE)_OBJS))
endif

define $(_MODULE)_PACKAGE

$(foreach lib,$($(_MODULE)_STATIC_LIBS),
$($(_MODULE)_PKG_LIB)/lib$(lib).a: $($(_MODULE)_TDIR)/lib$(lib).a $($(_MODULE)_PKG_LIB)/.gitignore
	$(Q)cp $$< $$@
)

$(foreach lib,$($(_MODULE)_SHARED_LIBS),
$($(_MODULE)_PKG_LIB)/lib$(lib).so: $($(_MODULE)_TDIR)/lib$(lib).so $($(_MODULE)_PKG_LIB)/.gitignore
	$(Q)cp $$< $$@
)

$(foreach bin,$($(_MODULE)_BINS),
$($(_MODULE)_PKG_BIN)/$(bin): $($(_MODULE)_TDIR)/$(bin) $($(_MODULE)_PKG_BIN)/.gitignore
	$(Q)cp $$< $$@
)

$(foreach inc,$($(_MODULE)_INCS),
$($(_MODULE)_PKG_INC)/$(notdir $(inc)): $(inc) $($(_MODULE)_PKG_INC)/.gitignore
	$(Q)cp $$< $$@
)

$($(_MODULE)_PKG_CFG)/$($(_MODULE)_CFG) : $($(_MODULE)_SDIR)/$($(_MODULE)_CFG) $($(_MODULE)_PKG_CFG)/.gitignore
	$(Q)echo "Package: $($(_MODULE)_PKG_NAME)" > $$@
	$(Q)cat $($(_MODULE)_SDIR)/$($(_MODULE)_CFG) >> $$@
	$(Q)echo "Architecture: $(DEB_BUILD_ARCH)" >> $$@

build:: $($(_MODULE)_BIN)

$($(_MODULE)_BIN): $($(_MODULE)_OBJS)
	$(Q)dpkg --build $$(basename $$@)
endef

else
# This prevents non-dpkg system from worrying about packages
$(_MODULE)_BIN :=
endif
endif

