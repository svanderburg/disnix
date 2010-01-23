AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-collect-garbage

disnix_collect_garbage_SOURCES = main.c
disnix_collect_garbage_LDADD = ../libinfrastructure/libinfrastructure.a
disnix_collect_garbage_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libinfrastructure
disnix_collect_garbage_LDFLAGS = $(LIBXML2_LIBS) $(LIBXSLT_LIBS)
