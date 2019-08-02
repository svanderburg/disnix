lib_LTLIBRARIES = libnixxml.la
include_HEADERS = nixxml-parse.h nixxml-print.h nixxml-print-nix.h nixxml-print-xml.h nixxml-node.h nixxml-parse-generic.h nixxml-print-generic-nix.h nixxml-print-generic-xml.h nixxml-generate-env.h nixxml-generate-env-generic.h nixxml-util.h

libnixxml_la_SOURCES = nixxml-parse.c nixxml-print.c nixxml-print-nix.c nixxml-print-xml.c nixxml-node.c nixxml-parse-generic.c nixxml-print-generic-nix.c nixxml-print-generic-xml.c nixxml-generate-env.c nixxml-generate-env-generic.c nixxml-util.c
libnixxml_la_CFLAGS = $(LIBXML2_CFLAGS)
libnixxml_la_LIBADD = $(LIBXML2_LIBS)
