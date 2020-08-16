pkglib_LTLIBRARIES = libstatemgmt.la
pkginclude_HEADERS = state-management.h snapshot-management.h remote-state-management.h remote-snapshot-management.h copy-snapshots.h

libstatemgmt_la_SOURCES = state-management.c snapshot-management.c remote-state-management.c remote-snapshot-management.c copy-snapshots.c
libstatemgmt_la_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact
libstatemgmt_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la
