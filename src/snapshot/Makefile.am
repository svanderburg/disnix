AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-snapshot
noinst_HEADERS = snapshot.h

disnix_snapshot_SOURCES = snapshot.c main.c
disnix_snapshot_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_snapshot_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
