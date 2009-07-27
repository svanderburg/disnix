AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-distribute

disnix_distribute_SOURCES = main.c
disnix_distribute_LDADD = ../libdistexport/libdistexport.a
disnix_distribute_CFLAGS = $(LIBXML2_CFLAGS) -I../libdistexport
disnix_distribute_LDFLAGS = $(LIBXML2_LIBS)
