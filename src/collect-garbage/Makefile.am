AM_CPPFLAGS = -ggdb -DDATADIR="\"$(prefix)/share/disnix\""

bin_PROGRAMS = disnix-collect-garbage

disnix_collect_garbage_SOURCES = main.c
disnix_collect_garbage_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS)
disnix_collect_garbage_LDFLAGS = $(LIBXML2_LIBS) $(LIBXSLT_LIBS)
