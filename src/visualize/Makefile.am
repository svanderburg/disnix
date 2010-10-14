AM_CPPFLAGS = -ggdb

bin_PROGRAMS = disnix-visualize
noinst_HEADERS = clustertable.h edgestable.h graph.h

disnix_visualize_SOURCES = main.c clustertable.c edgestable.c graph.c
disnix_visualize_LDADD = ../libmanifest/libmanifest.la
disnix_visualize_CFLAGS = -I../libmanifest
