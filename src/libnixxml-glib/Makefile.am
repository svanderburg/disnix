lib_LTLIBRARIES = libnixxml-glib.la
include_HEADERS = nixxml-gptrarray.h nixxml-ghashtable.h nixxml-ghashtable-iter.h nixxml-glib.h

libnixxml_glib_la_SOURCES = nixxml-gptrarray.c nixxml-ghashtable.c nixxml-ghashtable-iter.c nixxml-glib.c
libnixxml_glib_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libnixxml
libnixxml_glib_la_LIBADD = $(GLIB2_LIBS) ../libnixxml/libnixxml.la
