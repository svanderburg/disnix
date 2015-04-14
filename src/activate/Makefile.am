AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-activate
noinst_HEADERS = transition.h activate.h

disnix_activate_SOURCES = transition.c activate.c main.c
disnix_activate_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_activate_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
