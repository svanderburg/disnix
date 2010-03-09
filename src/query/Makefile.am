AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-query

disnix_query_SOURCES = main.c
disnix_query_LDADD = ../libinfrastructure/libinfrastructure.la
disnix_query_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libinfrastructure
disnix_query_LDFLAGS = $(LIBXML2_LIBS) $(LIBXSLT_LIBS)
