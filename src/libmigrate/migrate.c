#include "migrate.h"
#include "snapshot.h"
#include "restore.h"
#include "delete-state.h"

int migrate(const Manifest *manifest, const Manifest *previous_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    return (snapshot(manifest, previous_manifest, max_concurrent_transfers, flags, keep)
      && restore(manifest, previous_manifest, max_concurrent_transfers, flags, keep)
      && (!(flags & FLAG_DELETE_STATE) || (previous_manifest == NULL) || (flags & FLAG_NO_UPGRADE) || delete_obsolete_state(previous_manifest->snapshot_mapping_array, previous_manifest->services_table, manifest->targets_table)));
}
