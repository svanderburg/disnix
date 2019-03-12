pkglib_LTLIBRARIES = libdeploy.la
pkginclude_HEADERS = distribute.h locking.h profiles.h transition.h activate.h deploy.h deploymentflags.h

libdeploy_la_SOURCES = distribute.c locking.c profiles.c transition.c activate.c deploy.c
libdeploy_la_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libmanifest  -I../libmodel -I../libinterface -I../libpkgmgmt -I../libmigrate
libdeploy_la_LIBADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libmanifest/libmanifest.la ../libinterface/libinterface.la ../libpkgmgmt/libpkgmgmt.la ../libmigrate/libmigrate.la
