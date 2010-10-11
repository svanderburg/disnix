AM_CPPFLAGS = -ggdb -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-activate

disnix_activate_SOURCES = main.c
disnix_activate_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la
disnix_activate_CFLAGS = -I../libmanifest -I../libmain
