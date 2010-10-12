AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-collect-garbage

disnix_collect_garbage_SOURCES = main.c
disnix_collect_garbage_LDADD = ../libinfrastructure/libinfrastructure.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_collect_garbage_CFLAGS = $(LIBXML2_CFLAGS) $(LIBXSLT_CFLAGS) -I../libinfrastructure -I../libmain -I../libinterface
