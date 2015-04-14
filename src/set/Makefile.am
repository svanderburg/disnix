AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-set
noinst_HEADERS = profiles.h

disnix_set_SOURCES = profiles.c main.c
disnix_set_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_set_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
