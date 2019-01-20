#ifndef __DISNIX_MIGRATE_H
#define __DISNIX_MIGRATE_H
#include <glib.h>
#include <manifest.h>
#include "datamigrationflags.h"

/**
 * Migrates the state of all stateful services from the previous locations to
 * the desired locations.
 *
 * @param manifest Manifest containing all deployment information
 * @param old_snapshots_array Array of stateful components belonging to the previous configurations
 * @param max_concurrent_transfers Specifies the maximum amount of concurrent transfers
 * @param flags Data migration option flags
 * @param keep Indicates how many snapshot generations should be kept remotely while executing the depth first operation
 * @return TRUE if the migration completed successfully, else FALSE
 */
int migrate(const Manifest *manifest, GPtrArray *old_snapshots_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep);

#endif
