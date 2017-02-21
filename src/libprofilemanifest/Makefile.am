pkglib_LTLIBRARIES = libprofilemanifest.la
pkginclude_HEADERS = profilemanifest.h profilemanifesttarget.h

libprofilemanifest_la_SOURCES = profilemanifest.c profilemanifesttarget.c
libprofilemanifest_la_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact
libprofilemanifest_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la
