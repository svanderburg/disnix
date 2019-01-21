#include "migrate.h"
#include "snapshot.h"
#include "restore.h"
#include "delete-state.h"

int migrate(const Manifest *manifest, GPtrArray *old_snapshots_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    return (snapshot(manifest, old_snapshots_array, max_concurrent_transfers, flags, keep)
      && restore(manifest, old_snapshots_array, max_concurrent_transfers, flags, keep)
      && (!(flags & FLAG_DELETE_STATE) || (old_snapshots_array == NULL) || (flags & FLAG_NO_UPGRADE) || delete_obsolete_state(old_snapshots_array, manifest->target_array)));
}
