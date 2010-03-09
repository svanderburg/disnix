AM_CPPFLAGS = -ggdb -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-activate

disnix_activate_SOURCES = main.c
disnix_activate_LDADD = ../libmanifest/libmanifest.la
disnix_activate_CFLAGS = $(LIBXML2_CFLAGS) -I../libmanifest
disnix_activate_LDFLAGS = $(LIBXML2_LIBS)
