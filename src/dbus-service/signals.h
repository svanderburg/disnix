#ifndef __SIGNALS_H
#define __SIGNALS_H
#include <glib.h>
#include "disnix-instance.h"

gboolean disnix_emit_finish_signal(DisnixObject *obj, gchar *pid);

gboolean disnix_emit_success_signal(DisnixObject *obj, gchar *pid, gchar **derivation);

gboolean disnix_emit_failure_signal(DisnixObject *obj, gchar *pid);

#endif
