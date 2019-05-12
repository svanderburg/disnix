pkglib_LTLIBRARIES = libdistderivation.la
pkginclude_HEADERS = distributedderivation.h derivationmapping.h derivationmapping-iterator.h interfaces.h

libdistderivation_la_SOURCES = distributedderivation.c derivationmapping.c derivationmapping-iterator.c interfaces.c
libdistderivation_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel
libdistderivation_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libnixxml-glib/libnixxml-glib.la
