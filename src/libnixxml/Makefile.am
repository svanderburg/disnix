pkglib_LTLIBRARIES = libnixxml.la

pkginclude_HEADERS = nixxml-generate-env.h \
	nixxml-generate-env-generic.h \
	nixxml-node.h \
	nixxml-parse.h \
	nixxml-parse-generic.h \
	nixxml-print.h \
	nixxml-print-generic-nix.h \
	nixxml-print-generic-xml.h \
	nixxml-print-nix.h \
	nixxml-print-xml.h \
	nixxml-types.h \
	nixxml-util.h

libnixxml_la_SOURCES = 	nixxml-generate-env.c \
	nixxml-generate-env-generic.c \
	nixxml-node.c \
	nixxml-parse.c \
	nixxml-parse-generic.c \
	nixxml-print.c \
	nixxml-print-generic-nix.c \
	nixxml-print-generic-xml.c \
	nixxml-print-nix.c \
	nixxml-print-xml.c \
	nixxml-util.c

libnixxml_la_CFLAGS = $(LIBXML2_CFLAGS)
libnixxml_la_LIBADD = $(LIBXML2_LIBS)
