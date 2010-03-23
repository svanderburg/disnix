XMLLINT = $(xmllint) $(xmlflags)
XSLTPROC = $(xsltproc) $(xmlflags) \
 --param section.autolabel 1 \
 --param section.label.includes.component.label 1 \
 --param html.stylesheet \'style.css\' \
 --param xref.with.number.and.title 1 \
 --param toc.section.depth 3 \
 --param admon.style \'\' \
 --param callout.graphics.extension \'.gif\' \
 --param contrib.inline.enabled 0

dblatex_opts = \
 -P doc.collab.show=0 \
 -P latex.output.revhistory=0

MANUAL_SRCS = manual.xml introduction.xml installation.xml

# Note: RelaxNG validation requires xmllint >= 2.7.4.
manual.is-valid: $(MANUAL_SRCS) version.txt
	$(XMLLINT) --noout --nonet --xinclude --noxincludenode --relaxng $(docbookrng)/docbook.rng $<
	touch $@

manual.html: $(MANUAL_SRCS) manual.is-valid
	$(XSLTPROC) --nonet --xinclude --output manual.html \
	  $(docbookxsl)/html/docbook.xsl manual.xml

manual.pdf: $(MANUAL_SRCS) manual.is-valid
	if test "$(dblatex)" != ""; then \
		$(dblatex) $(dblatex_opts) manual.xml; \
	else \
		echo "Please install dblatex and rerun configure."; \
		exit 1; \
	fi

EXTRA_DIST = $(MANUAL_SRCS)
