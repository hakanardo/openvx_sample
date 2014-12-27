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

ifneq ($(filter $(TARGET_OS),LINUX CYGWIN DARWIN),)

PKG_EXT := .tar.bz2

$(_MODULE)_PKG_NAME    := $(subst _,-,$($(_MODULE)_TARGET))
$(_MODULE)_PKG_FLDR    := $($(_MODULE)_TDIR)/$($(_MODULE)_PKG_NAME)
$(_MODULE)_PKG         := $($(_MODULE)_PKG_NAME)_$(TARGET_CPU)_$(VERSION)$(PKG_EXT)
$(_MODULE)_BIN         := $($(_MODULE)_TDIR)/$($(_MODULE)_PKG)

# Remember that the INSTALL variable tend to be based in /
$(_MODULE)_PKG_LIB := $($(_MODULE)_PKG_FLDR)$($(_MODULE)_INSTALL_LIB)
$(_MODULE)_PKG_INC := $($(_MODULE)_PKG_FLDR)$($(_MODULE)_INSTALL_INC)/$($(_MODULE)_INC_SUBPATH)
$(_MODULE)_PKG_BIN := $($(_MODULE)_PKG_FLDR)$($(_MODULE)_INSTALL_BIN)

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

$(_MODULE)_OBJS := $($(_MODULE)_PKG_DEPS)

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

build:: $($(_MODULE)_BIN)

$($(_MODULE)_BIN): $($(_MODULE)_OBJS)
	$(Q)cd $($(_MODULE)_PKG_FLDR) && tar cjf $$(notdir $$@) $$(subst $($(_MODULE)_PKG_FLDR),.,$$^)
	@echo Packaging $$(notdir $$@)
endef

else
# This prevents non-tar system from worrying about packages
$(_MODULE)_BIN :=
endif

