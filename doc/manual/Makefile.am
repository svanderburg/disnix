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

FIGURES = model.png stafftracker.png disnixwinservice.png

MANUAL_SRCS = index.xml introduction.xml overview.xml installation.xml basicusage.xml dysnomia-modules.xml usage.xml advanced-options.xml images style.css

version.txt:
	echo -n $(VERSION) > version.txt

# Note: RelaxNG validation requires xmllint >= 2.7.4.
manual.is-valid: $(MANUAL_SRCS) version.txt
	$(XMLLINT) --noout --nonet --xinclude --noxincludenode --relaxng $(docbookrng)/docbook.rng $<
	touch $@

index.html: $(MANUAL_SRCS) manual.is-valid images
	$(XSLTPROC) --nonet --xinclude --output index.html \
	  $(docbookxsl)/html/docbook.xsl index.xml

index.pdf: $(MANUAL_SRCS) $(MAN_SRCS) manual.is-valid images
	if test "$(dblatex)" != ""; then \
		$(dblatex) $(dblatex_opts) index.xml; \
	else \
		echo "Please install dblatex and rerun configure."; \
		exit 1; \
	fi

images:
	mkdir images
	mkdir images/callouts
	cp $(docbookxsl)/images/callouts/*.gif images/callouts
	chmod -R +w images

all-local: index.html

install-data-local: index.html
	$(INSTALL) -d $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) index.html $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) style.css $(DESTDIR)$(docdir)/manual
	$(INSTALL_DATA) $(FIGURES) $(DESTDIR)$(docdir)/manual
	cp -r images $(DESTDIR)$(docdir)/manual/images

KEEP = index.html manual.is-valid version.txt

EXTRA_DIST = $(MANUAL_SRCS) $(FIGURES) $(KEEP)

DISTCLEANFILES = $(KEEP)
