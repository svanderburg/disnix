pkglib_LTLIBRARIES = libmodel.la
pkginclude_HEADERS = modeliterator.h hashtable-util.h

libmodel_la_SOURCES = modeliterator.c hashtable-util.c
libmodel_la_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB2_CFLAGS) -I../libprocreact
libmodel_la_LIBADD = $(LIBXML2_LIBS) $(GLIB2_LIBS) ../libprocreact/libprocreact.la
