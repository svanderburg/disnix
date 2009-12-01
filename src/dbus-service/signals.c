#include "signals.h"
#include "disnix-gtype-def.h"
#include <glib-object.h>

gboolean disnix_emit_finish_signal(DisnixObject *obj, gchar *pid)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_FINISH_SIGNAL], 0, pid);
    return TRUE;
}

gboolean disnix_emit_success_signal(DisnixObject *obj, gchar *pid, gchar **derivation)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_SUCCESS_SIGNAL], 0, pid, derivation);
    return TRUE;
}

gboolean disnix_emit_failure_signal(DisnixObject *obj, gchar *pid)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_FAILURE_SIGNAL], 0, pid);
    return TRUE;
}
