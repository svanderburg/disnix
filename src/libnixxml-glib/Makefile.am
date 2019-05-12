pkglib_LTLIBRARIES = libnixxml-glib.la
pkginclude_HEADERS = nixxml-gptrarray.h nixxml-ghashtable.h

libnixxml_glib_la_SOURCES = nixxml-gptrarray.c nixxml-ghashtable.c
libnixxml_glib_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libnixxml
libnixxml_glib_la_LIBADD = $(GLIB2_LIBS) ../libnixxml/libnixxml.la
