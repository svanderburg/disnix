disnix-build.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Build store derivations on target machines in a network' --libtool ./disnix-build

disnix-build.1.xml: disnix-build.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

bin_PROGRAMS = disnix-build
noinst_HEADERS = run-build.h
noinst_DATA = disnix-build.1.xml
man1_MANS = disnix-build.1

disnix_build_SOURCES = run-build.c main.c
disnix_build_CFLAGS = $(GLIB2_CFLAGS) -I../libprocreact -I../libdistderivation -I../libmain -I../libinterface -I../libmodel -I../libbuild
disnix_build_LDADD = $(GLIB2_LIBS) ../libprocreact/libprocreact.la ../libdistderivation/libdistderivation.la ../libmain/libmain.la ../libinterface/libinterface.la ../libbuild/libbuild.la

EXTRA_DIST = $(man1_MANS) $(noinst_DATA)
