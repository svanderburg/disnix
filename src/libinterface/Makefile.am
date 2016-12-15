pkglib_LTLIBRARIES = libinterface.la
pkginclude_HEADERS = client-interface.h

libinterface_la_SOURCES = client-interface.c
libinterface_la_CFLAGS = -I../libprocreact $(GLIB2_CFLAGS)
libinterface_la_LIBADD = ../libprocreact/libprocreact.la $(GLIB2_LIBS)
