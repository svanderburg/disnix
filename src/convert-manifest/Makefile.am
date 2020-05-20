AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\" -DDATADIR=\"$(datadir)/$(PACKAGE)\"

disnix-convert-manifest.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Converts a manifest file using the old V1 schema into V2 format' --libtool ./disnix-convert-manifest

disnix-convert-manifest.1.xml: disnix-convert-manifest.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-convert-manifest
noinst_HEADERS = activationmapping.h convert-manifest.h distributionmapping.h oldmanifest.h oldsnapshotmapping.h
noinst_DATA = disnix-convert-manifest.1.xml
man1_MANS = disnix-convert-manifest.1

disnix_convert_manifest_SOURCES = activationmapping.c convert-manifest.c distributionmapping.c oldmanifest.c oldsnapshotmapping.c main.c
disnix_convert_manifest_LDADD = ../libprocreact/libprocreact.la ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libmanifest/libmanifest.la
disnix_convert_manifest_CFLAGS = $(GLIB2_CFLAGS) $(LIBXML2_CFLAGS) -I../libprocreact -I../libnixxml -I../libnixxml-glib -I../libinfrastructure -I../libmain -I../libmodel -I../libmanifest

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
