AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\" -DDATADIR=\"$(datadir)/$(PACKAGE)\"

disnix-capture-manifest.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Reconstructs the coordinator manifest from the manifest of the target profiles' --libtool ./disnix-capture-manifest

disnix-capture-manifest.1.xml: disnix-capture-manifest.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-capture-manifest
noinst_HEADERS = capture-manifest.h
noinst_DATA = disnix-capture-manifest.1.xml
man1_MANS = disnix-capture-manifest.1

disnix_capture_manifest_SOURCES = capture-manifest.c main.c
disnix_capture_manifest_LDADD = ../libprocreact/libprocreact.la ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libinterface/libinterface.la ../libprofilemanifest/libprofilemanifest.la ../libpkgmgmt/libpkgmgmt.la
disnix_capture_manifest_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libinfrastructure -I../libmain -I../libinterface -I../libxmlutil -I../libprofilemanifest -I../libpkgmgmt

EXTRA_DIST = disnix-capture-manifest.1 disnix-capture-manifest.1.xml
