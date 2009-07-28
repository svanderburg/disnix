AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-activate

disnix_activate_SOURCES = main.c
disnix_activate_LDADD = ../libdistexport/libdistexport.a
disnix_activate_CFLAGS = $(LIBXML2_CFLAGS) -I../libdistexport
disnix_activate_LDFLAGS = $(LIBXML2_LIBS)
