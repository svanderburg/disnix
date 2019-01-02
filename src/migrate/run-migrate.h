#ifndef __DISNIX_RUN_MIGRATE_H
#define __DISNIX_RUN_MIGRATE_H
#include <glib.h>
#include <datamigrationflags.h>

int run_migrate(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gchar *container_filter, const gchar *component_filter);

#endif
