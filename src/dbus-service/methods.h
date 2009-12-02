#ifndef __METHODS_H
#define __METHODS_H
#include <glib.h>
#include "disnix-instance.h"

gboolean disnix_import(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_export(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_print_invalid(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_realise(DisnixObject *object, gchar **derivation, gchar **pid, GError **error);

gboolean disnix_set(DisnixObject *object, const gchar *profile, const gchar *derivation, gchar **pid, GError **error);

gboolean disnix_query_installed(DisnixObject *object, const gchar *profile, gchar **pid, GError **error);

gboolean disnix_collect_garbage(DisnixObject *object, const gboolean delete_old, gchar **pid, GError **error);

gboolean disnix_activate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gchar **pid, GError **error);

gboolean disnix_deactivate(DisnixObject *object, const gchar *derivation, const gchar *type, gchar **arguments, gchar **pid, GError **error);

gboolean disnix_lock(DisnixObject *object, gchar **pid, GError **error);

gboolean disnix_unlock(DisnixObject *object, gchar **pid, GError **error);

#endif
