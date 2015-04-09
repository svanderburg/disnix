AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-restore
noinst_HEADERS = restore.h

disnix_restore_SOURCES = restore.c main.c
disnix_restore_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_restore_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
