pkglib_LTLIBRARIES = libbuild.la
pkginclude_HEADERS = build.h

libbuild_la_SOURCES = build.c
libbuild_la_CFLAGS = $(LIBXML2_CFLAGS) $(GLIB2_CFLAGS) -I../libnixxml -I../libprocreact -I../libdistderivation -I../libmodel -I../libpkgmgmt
libbuild_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libdistderivation/libdistderivation.la ../libpkgmgmt/libpkgmgmt.la
