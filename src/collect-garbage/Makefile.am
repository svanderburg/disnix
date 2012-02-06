bin_PROGRAMS = disnix-collect-garbage
noinst_HEADERS = collect-garbage.h

disnix_collect_garbage_SOURCES = collect-garbage.c main.c
disnix_collect_garbage_LDADD = ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_collect_garbage_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libinfrastructure -I../libmain -I../libinterface
