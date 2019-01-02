#ifndef __DISNIX_MIGRATE_H
#define __DISNIX_MIGRATE_H
#include <glib.h>
#include <manifest.h>
#include "datamigrationflags.h"

int migrate(const Manifest *manifest, GPtrArray *old_snapshots_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep);

#endif
