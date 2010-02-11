AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-distribute

disnix_distribute_SOURCES = main.c
disnix_distribute_LDADD = ../libmanifest/libmanifest.a ../libxmlutil/libxmlutil.a
disnix_distribute_CFLAGS = $(LIBXML2_CFLAGS) -I../libmanifest
disnix_distribute_LDFLAGS = $(LIBXML2_LIBS)
