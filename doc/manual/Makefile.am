XMLLINT = $(xmllint)
XSLTPROC = $(xsltproc) \
 --param section.autolabel 1 \
 --param section.label.includes.component.label 1 \
 --param html.stylesheet \'style.css\' \
 --param make.valid.html 1 \
 --param xref.with.number.and.title 1 \
 --param toc.section.depth 3 \
 --param admon.style \'\' \
 --param callout.graphics.extension \'.gif\' \
 --param contrib.inline.enabled 0

dblatex_opts = \
 -P doc.collab.show=0 \
 -P latex.output.revhistory=0

FIGURES = model.png stafftracker.png

MANUAL_SRCS = manual.xml introduction.xml overview.xml installation.xml models.xml activation-scripts.xml usage.xml images style.css

version.txt:
	echo -n $(VERSION) > version.txt

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

images:
	mkdir images
	mkdir images/callouts
	cp $(docbookxsl)/images/callouts/*.gif images/callouts
	chmod -R +w images

all-local: manual.html

install-data-local: manual.html
	$(INSTALL) -d $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) manual.html $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) style.css $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) $(FIGURES) $(DESTDIR)$(docdir)/manual
	cp -r images $(DESTDIR)$(docdir)/manual/images

KEEP = manual.html manual.is-valid version.txt

EXTRA_DIST = $(MANUAL_SRCS) $(FIGURES) $(KEEP)

DISTCLEANFILES = $(KEEP)
