disnix-compare-manifest.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Parse and compare two manifest files' --libtool ./disnix-compare-manifest

disnix-compare-manifest.1.xml: disnix-compare-manifest.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-compare-manifest
noinst_HEADERS = compare-manifest.h
noinst_DATA = disnix-compare-manifest.1.xml
man1_MANS = disnix-compare-manifest.1

disnix_compare_manifest_SOURCES = compare-manifest.c main.c
disnix_compare_manifest_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libmanifest -I../libmain
disnix_compare_manifest_LDADD = ../libmain/libmain.la ../libmanifest/libmanifest.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
