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

ifeq ($($(_MODULE)_TYPE),dsmo)

$(_MODULE)_INSTALL_DSO   := $(COPY) $(call PATH_CONV,$($(_MODULE)_TDIR)\\$($(_MODULE)_BIN)) $(call PATH_CONV,$($(_MODULE)_INSTALL_LIB))
$(_MODULE)_UNINSTALL_DSO := $(CLEAN) $(call PATH_CONV,$($(_MODULE)_INSTALL_LIB)\\$($(_MODULE)_OUT))
$(_MODULE)_LN_DS0        := $(SET_RW) $(call PATH_CONV,$($(_MODULE)_INSTALL_LIB)\\$($(_MODULE)_OUT))
$(_MODULE)_UNLN_DSO      := $(SET_RW) $(call PATH_CONV,$($(_MODULE)_INSTALL_LIB)\\$($(_MODULE)_OUT))

define $(_MODULE)_UNINSTALL
uninstall::
	@echo Uninstalling $($(_MODULE)_BIN) from $($(_MODULE)_INSTALL_LIB)
	-$(Q)$(call $(_MODULE)_UNLN_DSO)
	-$(Q)$(call $(_MODULE)_UNINSTALL_DSO)
endef

define $(_MODULE)_INSTALL
install:: $($(_MODULE)_BIN)
	@echo Installing $($(_MODULE)_BIN) to $($(_MODULE)_INSTALL_LIB)
	-$(Q)$(call $(_MODULE)_INSTALL_DSO)
	-$(Q)$(call $(_MODULE)_LN_DSO)
endef

# Windows does not support soft-linking versions
define $(_MODULE)_CLEAN_LNK
clean::
endef

else ifeq ($($(_MODULE)_TYPE),exe)

$(_MODULE)_ATTRIB_EXE := $(ATTRIB) $(call PATH_CONV,$($(_MODULE)_BIN))
$(_MODULE)_INSTALL_EXE := $(COPY) $(call PATH_CONV,$($(_MODULE)_TDIR)\\$($(_MODULE)_BIN)) $(call PATH_CONV,$($(_MODULE)_INSTALL_BIN))
$(_MODULE)_UNINSTALL_EXE := $(CLEAN) $(call PATH_CONV,$($(_MODULE)_INSTALL_BIN)\\$($(_MODULE)_BIN))

define $(_MODULE)_UNINSTALL
uninstall::
	@echo Uninstalling $($(_MODULE)_BIN) from $($(_MODULE)_INSTALL_BIN)
	-$(Q)$(call $(_MODULE)_UNINSTALL_EXE)
endef

define $(_MODULE)_INSTALL
install:: $($(_MODULE)_BIN)
	@echo Installing $($(_MODULE)_BIN) to $($(_MODULE)_INSTALL_BIN)
	-$(Q)$(call $(_MODULE)_INSTALL_EXE)
	-$(Q)$(call $(_MODULE)_ATTRIB_EXE)
endef

endif

