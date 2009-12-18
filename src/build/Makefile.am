AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-build

disnix_build_SOURCES = main.c
disnix_build_LDADD = ../libdistderivation/libdistderivation.a
disnix_build_CFLAGS = $(LIBXML2_CFLAGS) -I../libdistderivation
disnix_build_LDFLAGS = $(LIBXML2_LIBS)
