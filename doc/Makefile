SPHINXOPTS  ?=
SPHINXBUILD ?= sphinx-build
SOURCEDIR    = .
BUILDDIR     = _build

.PHONY: help html clean doxygen

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

html:
	@$(SPHINXBUILD) -b html "$(SOURCEDIR)" "$(BUILDDIR)/html" $(SPHINXOPTS) $(O)
	@echo ""
	@echo "Documentation générée : $(BUILDDIR)/html/index.html"

doxygen:
	cd .. && doxygen Doxyfile.sphinx

clean:
	rm -rf $(BUILDDIR) _doxygen

%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
