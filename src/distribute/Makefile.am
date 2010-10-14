AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-distribute
noinst_HEADERS = distribute.h

disnix_distribute_SOURCES = distribute.c main.c
disnix_distribute_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_distribute_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
