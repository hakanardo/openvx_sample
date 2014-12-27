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

ifeq ($(SCM_ROOT),)
# wildcard is required for windows since realpath doesn't return NULL if not found
SCM_ROOT := $(wildcard $(realpath .svn))
ifneq ($(SCM_ROOT),)
ifeq ($(SHOW_MAKEDEBUG),1)
$(info $(SCM_ROOT))
$(info Subversion is used)
endif
SCM_VERSION := r$(word 2, $(shell svn info | grep Revision))
endif
endif

ifeq ($(SCM_ROOT),)
# wildcard is required for windows since realpath doesn't return NULL if not found
SCM_ROOT := $(wildcard $(realpath .git))
ifneq ($(SCM_ROOT),)
ifeq ($(SHOW_MAKEDEBUG),1)
$(info $(SCM_ROOT))
$(info GIT is used)
endif
SCM_VERSION := $(shell git describe --tags --dirty --always)
# The following is in case git is not executable in the shell where the build is taking place (eg. DOS)
ifeq ($(SCM_VERSION),)
TEMP := $(lastword $(shell $(CAT) $(call PATH_CONV,"$(SCM_ROOT)/HEAD")))
SCM_VERSION := $(shell $(CAT) $(call PATH_CONV,"$(SCM_ROOT)/$(TEMP)"))
endif
endif
endif

