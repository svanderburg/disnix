pkglib_LTLIBRARIES = libdistderivation.la
pkginclude_HEADERS = distributedderivation.h derivationmapping.h derivationmappingarray.h derivationmapping-iterator.h interface.h interfacestable.h

libdistderivation_la_SOURCES = distributedderivation.c derivationmapping.c derivationmappingarray.c derivationmapping-iterator.c interface.c interfacestable.c
libdistderivation_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libmodel
libdistderivation_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmodel/libmodel.la ../libnixxml-glib/libnixxml-glib.la
