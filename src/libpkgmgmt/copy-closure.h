#ifndef __DISNIX_ACTIVATE_H
#define __DISNIX_ACTIVATE_H
#include <glib.h>

int copy_closure_to_sync(gchar *interface, gchar *target, gchar *tmpdir, gchar **derivation);

pid_t copy_closure_to(gchar *interface, gchar *target, gchar *tmpdir, gchar **derivation);

int copy_closure_from_sync(gchar *interface, gchar *target, gchar **derivation);

pid_t copy_closure_from(gchar *interface, gchar *target, gchar **derivation);

#endif
