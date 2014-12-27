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

DOXYGEN := doxygen
DOXYFILE ?= Doxyfile

PDFNAME ?= refman.pdf

$(_MODULE)_DOXYFILE := $($(_MODULE)_SDIR)/$(DOXYFILE)
$(_MODULE)_DOXYFILE_MOD := $(TARGET_DOC)/$(_MODULE)/$(DOXYFILE)
$(_MODULE)_BIN := $($(_MODULE)_TDIR)/$($(_MODULE)_TARGET).tar.gz

$(_MODULE)_HTML := $(TARGET_DOC)/$(_MODULE)/html/index.html
$(_MODULE)_TEX  := $(TARGET_DOC)/$(_MODULE)/latex/refman.tex
$(_MODULE)_PDFNAME := $(PDFNAME)
$(_MODULE)_PDF := $(TDIR)/$($(_MODULE)_PDFNAME)

#$(info Modified Doxyfile should be in $($(_MODULE)_DOXYFILE_MOD))

define $(_MODULE)_DOCUMENTS

$(TARGET_DOC)/$(_MODULE)/.gitignore:
	$(Q)$(MKDIR) $(TARGET_DOC)/$(_MODULE)

$($(_MODULE)_DOXYFILE_MOD): $($(_MODULE)_DOXYFILE) $(TARGET_DOC)/$(_MODULE)/.gitignore
	$(Q)$(COPY) $($(_MODULE)_DOXYFILE) $(TARGET_DOC)/$(_MODULE)
	$(Q)$(PRINT) OUTPUT_DIRECTORY=$(TARGET_DOC)/$(_MODULE) >> $($(_MODULE)_DOXYFILE_MOD)
	$(Q)$(PRINT) WARN_LOGFILE=$(TARGET_DOC)/$(_MODULE)/warnings.txt >> $($(_MODULE)_DOXYFILE_MOD)
	$(Q)$(PRINT) GENERATE_TAGFILE=$(TARGET_DOC)/$(_MODULE)/$(_MODULE).tags >> $($(_MODULE)_DOXYFILE_MOD)  
ifneq ($(SCM_VERSION),)
	-$(Q)$(PRINT) PROJECT_NUMBER=$(SCM_VERSION) >> $($(_MODULE)_DOXYFILE_MOD)
endif

$($(_MODULE)_HTML): $($(_MODULE)_DOXYFILE_MOD)
	$(Q)$(CLEANDIR) $(TARGET_DOC)/$(_MODULE)/html
	$(Q)$(DOXYGEN) $($(_MODULE)_DOXYFILE_MOD)

$($(_MODULE)_TEX): $($(_MODULE)_DOXYFILE_MOD)
	$(Q)$(CLEANDIR) $(TARGET_DOC)/$(_MODULE)/latex
	$(Q)$(DOXYGEN) $($(_MODULE)_DOXYFILE_MOD)

$($(_MODULE)_PDF): $($(_MODULE)_TEX)
	-$(Q)cp docs/images/* $(TARGET_DOC)/$(_MODULE)/latex/; \
	 cd $(TARGET_DOC)/$(_MODULE)/latex/; \
	 pdflatex refman; \
	 bibtex refman; \
	 makeindex refman.idx; \
	 pdflatex refman; 
	-$(Q)cd $(TARGET_DOC)/$(_MODULE)/latex; cp refman.pdf $($(_MODULE)_PDFNAME); cp refman.pdf $($(_MODULE)_PDF)

$(_MODULE)_docs:: $($(_MODULE)_HTML) $($(_MODULE)_PDF) $($(_MODULE)_ODIR)/.gitignore
	$(Q)tar zcf $($(_MODULE)_BIN) $(TARGET_DOC)/$(_MODULE)/html

$($(_MODULE)_BIN):: $(_MODULE)_docs

docs:: $(_MODULE)_docs
	$(Q)echo Building docs for $(_MODULE)

clean_docs::
	$(Q)$(CLEANDIR) docs/$(_MODULE)/latex
	$(Q)$(CLEANDIR) docs/$(_MODULE)/html

endef


