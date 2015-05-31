disnix-restore.1: main.c
	$(HELP2MAN) --output=$@ --no-info --name 'Restores the state of components' --libtool ./disnix-restore

disnix-restore.1.xml: disnix-restore.1
	$(SHELL) ../../maintenance/man2docbook.bash $<

AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-restore
noinst_HEADERS = restore.h
noinst_DATA = disnix-restore.1.xml
man1_MANS = disnix-restore.1

disnix_restore_SOURCES = restore.c main.c
disnix_restore_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_restore_CFLAGS = -I../libmanifest -I../libmain -I../libinterface

EXTRA_DIST = disnix-restore.1 disnix-restore.1.xml
