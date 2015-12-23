disnix-set.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Updates the coordinator and target Nix profiles' --libtool ./disnix-set

disnix-set.1.xml: disnix-set.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-set
noinst_HEADERS = profiles.h
noinst_DATA = disnix-set.1.xml
man1_MANS = disnix-set.1

disnix_set_SOURCES = profiles.c main.c
disnix_set_CFLAGS = $(GLIB2_CFLAGS) -I../libmanifest -I../libmain -I../libinterface
disnix_set_LDADD = $(GLIB2_LIBS) ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la

EXTRA_DIST = disnix-set.1 disnix-set.1.xml
