AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-build
noinst_HEADERS = build.h

disnix_build_SOURCES = build.c main.c
disnix_build_LDADD = ../libdistderivation/libdistderivation.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_build_CFLAGS = -I../libdistderivation -I../libmain -I../libinterface
