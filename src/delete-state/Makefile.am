AM_CPPFLAGS = -DLOCALSTATEDIR=\"$(localstatedir)\"

bin_PROGRAMS = disnix-delete-state
noinst_HEADERS = delete-state.h

disnix_delete_state_SOURCES = delete-state.c main.c
disnix_delete_state_LDADD = ../libmanifest/libmanifest.la ../libmain/libmain.la ../libinterface/libinterface.la
disnix_delete_state_CFLAGS = -I../libmanifest -I../libmain -I../libinterface
