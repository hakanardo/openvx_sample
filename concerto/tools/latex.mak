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

# Tools
LATEX := latex
PDFLATEX := pdflatex
MSCGEN := mscgen
DOT := dot
EPSTOPDF := epstopdf
BIBTEX := bibtex

PDFNAME ?= $(TARGET).pdf

BIB_OBJS := $(addprefix $(ODIR)/,$(BIBFILES))
DOT_OBJS := $(DOTFILES:%.dot=$(ODIR)/%.pdf)
MSC_OBJS := $(MSCFILES:%.msc=$(ODIR)/%.pdf)
TEX_OBJS := $(TEXFILES:%.tex=$(ODIR)/%.pdf)
STY_OBJS := $(addprefix $(ODIR)/,$(STYFILES))
BST_OBJS := $(addprefix $(ODIR)/,$(BSTFILES))
IMG_OBJS := $(addprefix $(ODIR)/,$(IMGFILES))

$(_MODULE)_SUPPORT := $(BIBFILES) $(BSTFILES) $(STYFILES)
$(_MODULE)_SUPPORT_SRCS := $(foreach sup,$(SUPPORT),$(SDIR)/$(sup))
$(_MODULE)_SUPPORT_OBJS := $(foreach sup,$(SUPPORT),$(ODIR)/$(sup))

$(_MODULE)_BIN := $(TDIR)/$(PDFNAME)
$(_MODULE)_SRCS := $(DOTFILES) $(MSCFILES) $(TEXFILES) $(BIBFILES) $(BSTFILES) $(STYFILES)
$(_MODULE)_OBJS := $(DOT_OBJS) $(MSC_OBJS) $(BIB_OBJS) $(TEX_OBJS) $(STY_OBJS) $(BST_OBJS) $(IMG_OBJS)
# in case the name gets duplicated 
$(_MODULE)_OBJS := $(filter-out $(ODIR)/$(PDFNAME),$($(_MODULE)_OBJS))

ifeq ($(SHOW_COMMANDS),1)
DFLAGS := --halt-on-error
endif

ifeq ($(SHOW_MAKEDEBUG),1)
$(info Latex SUPPORT_OBJS=$(SUPPORT_OBJS))
endif

define $(_MODULE)_COMPILE_TOOLS

$($(_MODULE)_BIN): $(ODIR)/$(PDFNAME) $(TDIR)/.gitignore
	@echo [COPY] $$(notdir $$<)
	$(Q)$(COPY) $$< $$@

$(ODIR)/%.png: $(SDIR)/%.png $(ODIR)/.gitignore
	@echo [COPY] $$(notdir $$<)
	$(Q)$(COPY) $$< $$@	

$(ODIR)/%.pdf: $(SDIR)/%.dot $(ODIR)/.gitignore
	@echo [DOT] $$(notdir $$<)
ifeq ($(USE_SVG),true)	
	$(Q)$(DOT) -T svg -o $(ODIR)/$$(notdir $$(basename $$<)).svg $$<
	$(Q)xsltproc -o $(ODIR)/$$(notdir $$(basename $$<))_new.svg $(CONCERTO_ROOT)/tools/notuglydot.xsl $(ODIR)/$$(notdir $$(basename $$<)).svg 
	$(Q)inkscape -z -D --file $(ODIR)/$$(notdir $$(basename $$<))_new.svg --export-pdf=$$@
else	
	$(Q)$(DOT) -T pdf -o $$@ $$<
endif

$(ODIR)/%.eps: $(SDIR)/%.msc $(ODIR)/.gitignore
	@echo [MSC] $$(notdir $$<)
	$(Q)$(MSCGEN) -T eps -i $$< -o $$(basename $$@).eps

$(ODIR)/%.pdf: $(ODIR)/%.eps $(ODIR)/.gitignore
	@echo [PDF] Convert $$(notdir $$<)
	$(Q)$(EPSTOPDF) $$< --outfile=$$@

$(ODIR)/%.tex: $(SDIR)/%.tex $(ODIR)/.gitignore
	@echo [COPY] $$(notdir $$<)
	$(Q)$(COPY) $$< $$@

$(ODIR)/%.sty: $(SDIR)/%.sty $(ODIR)/.gitignore
	@echo [COPY] $$(notdir $$<)
	$(Q)$(COPY) $$< $$@

$(ODIR)/%.bst: $(SDIR)/%.bst $(ODIR)/.gitignore
	@echo [COPY] $$(notdir $$<)
	$(Q)$(COPY) $$< $$@

$(ODIR)/%.bib: $(SDIR)/%.bib $(ODIR)/.gitignore $(ODIR)/.gitignore
	@echo [COPY] $$(notdir $$<)
	$(Q)$(COPY) $$< $$@

$(ODIR)/%.pdf: $(ODIR)/%.tex $($(_MODULE)_OBJS) $(ODIR)/.gitignore
	@echo [TEX] $$(notdir $$<)
	$(Q)cd $(ODIR);$(PDFLATEX) $(DFLAGS) -shell-escape -output-format=pdf --output-directory=$(ODIR) $$<
	@echo [BIB] Fixing bibliography
	$(Q)cd $(ODIR);$(BIBTEX) $$(basename $$(notdir $$<))
	@echo [TEX] $$(notdir $$<) Post-bib-fixup
	$(Q)cd $(ODIR);$(PDFLATEX) $(DFLAGS) -shell-escape -output-format=pdf --output-directory=$(ODIR) $$<
	$(Q)cd $(ODIR);$(PDFLATEX) $(DFLAGS) -shell-escape -output-format=pdf --output-directory=$(ODIR) $$<

$(ODIR)/$(PDFNAME): $(TEX_OBJS)
	$(Q)$(COPY) $$< $$@

latex:: $($(_MODULE)_BIN)

clean_latex::
	$(Q)rm $($(_MODULE)_OBJS) $($(_MODULE)_BIN)
 
endef
