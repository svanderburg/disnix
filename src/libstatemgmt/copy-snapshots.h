#ifndef __DISNIX_COPYSNAPSHOTS_H
#define __DISNIX_COPYSNAPSHOTS_H
#include <glib.h>
#include <sys/types.h>

int copy_snapshots_to_sync(gchar *interface, gchar *target, gchar *container, gchar *component, int all);

pid_t copy_snapshots_to(gchar *interface, gchar *target, gchar *container, gchar *component, int all);

int copy_snapshots_from_sync(gchar *interface, gchar *target, gchar *container, gchar *component, int all);

pid_t copy_snapshots_from(gchar *interface, gchar *target, gchar *container, gchar *component, int all);

#endif
