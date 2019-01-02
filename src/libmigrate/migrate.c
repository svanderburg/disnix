#include "migrate.h"
#include "snapshotmapping.h"
#include "snapshot.h"
#include "restore.h"
#include "delete-state.h"

static int clear_and_delete_obsolete_state(GPtrArray *old_snapshots_array, GPtrArray *target_array, const unsigned int flags)
{
    if((flags & FLAG_DELETE_STATE) && (old_snapshots_array != NULL) && !(flags & FLAG_NO_UPGRADE))
    {
        clear_snapshot_items_transferred_status(old_snapshots_array);
        return delete_obsolete_state(old_snapshots_array, target_array);
    }
    else
        return TRUE;
}

int migrate(const Manifest *manifest, GPtrArray *old_snapshots_array, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep)
{
    return (snapshot(manifest, old_snapshots_array, max_concurrent_transfers, flags, keep)
      && restore(manifest, old_snapshots_array, max_concurrent_transfers, flags, keep)
      && clear_and_delete_obsolete_state(old_snapshots_array, manifest->target_array, flags));
}
