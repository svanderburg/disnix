AM_CPPFLAGS = -ggdb -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-activate
noinst_HEADERS = transition.h locking.h profiles.h activate.h

disnix_activate_SOURCES = transition.c locking.c profiles.c activate.c main.c
disnix_activate_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_activate_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
