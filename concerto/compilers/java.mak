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

ifeq ($($(_MODULE)_TYPE),jar)

ifeq ($(SHOW_MAKEDEBUG),1)
$(info Building JAR file $(_MODULE))
endif

JAR := jar
JC  := javac

EMPTY:=
SPACE:=$(EMPTY) $(EMPTY)

###############################################################################

$(_MODULE)_BIN       := $($(_MODULE)_TDIR)/$($(_MODULE)_TARGET).jar
$(_MODULE)_CLASSES   := $(patsubst %.java,%.class,$(JSOURCES))
$(_MODULE)_OBJS      := $(foreach cls,$($(_MODULE)_CLASSES),$($(_MODULE)_ODIR)/$(cls))
ifdef CLASSPATH
$(_MODULE)_CLASSPATH := $(CLASSPATH) $($(_MODULE)_ODIR)
CLASSPATH            :=
else
$(_MODULE)_CLASSPATH := $($(_MODULE)_ODIR)
endif
ifneq ($($(_MODULE)_JAVA_LIBS),)
$(_MODULE)_JAVA_DEPS := $(foreach lib,$($(_MODULE)_JAVA_LIBS),$($(_MODULE)_TDIR)/$(lib).jar)
$(_MODULE)_CLASSPATH += $($(_MODULE)_JAVA_DEPS)
endif

ifeq ($(SHOW_MAKEDEBUG),1)
$(info CLASSPATH=$($(_MODULE)_CLASSPATH))
endif
$(_MODULE)_CLASSPATH := $(subst $(SPACE),:,$($(_MODULE)_CLASSPATH))
JC_OPTS              := -deprecation -classpath $($(_MODULE)_CLASSPATH) -sourcepath $($(_MODULE)_SDIR) -d $($(_MODULE)_ODIR)
ifeq ($(TARGET_BUILD),debug)
JC_OPTS              += -g -verbose
else ifneq ($(filter $(TARGET_BUILD),release production),)
# perform obfuscation?
endif

ifdef MANIFEST
$(_MODULE)_MANIFEST  := -m $(MANIFEST)
MANIFEST             :=
else
$(_MODULE)_MANIFEST  :=
endif
ifdef ENTRY
$(_MODULE)_ENTRY     := $(ENTRY)
ENTRY                :=
else
$(_MODULE)_ENTRY     := Main
endif
JAR_OPTS             := cvfe $($(_MODULE)_BIN) $($(_MODULE)_MANIFEST) $($(_MODULE)_ENTRY) $(foreach cls,$($(_MODULE)_CLASSES),-C $($(_MODULE)_ODIR) $(cls))

###############################################################################


define $(_MODULE)_DEPEND_CLS
$($(_MODULE)_ODIR)/$(1).class: $($(_MODULE)_SDIR)/$(1).java $($(_MODULE)_SDIR)/$(SUBMAKEFILE) $($(_MODULE)_JAVA_DEPS) $($(_MODULE)_ODIR)/.gitignore
	@echo Compiling Java $(1).java
	$(Q)$(JC) $(JC_OPTS) $($(_MODULE)_SDIR)/$(1).java
endef

define $(_MODULE)_DEPEND_JAR
uninstall::
build:: $($(_MODULE)_BIN)
	@echo Building for $($(_MODULE)_BIN)
install:: $($(_MODULE)_BIN)

$($(_MODULE)_BIN): $($(_MODULE)_OBJS) $($(_MODULE)_SDIR)/$(SUBMAKEFILE)
	@echo Jar-ing Classes $($(_MODULE)_CLASSES)
	$(Q)$(JAR) $(JAR_OPTS) $(foreach cls,$($(_MODULE)_CLASSES),-C $($(_MODULE)_ODIR) $(cls))
endef

endif

