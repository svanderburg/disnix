#include "run-migrate.h"
#include <migrate.h>
#include <manifest.h>
#include <snapshotmappingarray.h>

int run_migrate(const gchar *manifest_file, const unsigned int max_concurrent_transfers, const unsigned int flags, const int keep, const gchar *old_manifest, const gchar *coordinator_profile_path, gchar *profile, const gchar *container_filter, const gchar *component_filter)
{
    /* Generate a distribution array from the manifest file */
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_FLAG | MANIFEST_TARGETS_FLAG, container_filter, component_filter);

    if(manifest == NULL)
    {
        g_print("[coordinator]: Error while opening manifest file!\n");
        return 1;
    }
    else
    {
        int exit_status;

        if(check_manifest(manifest))
        {
            Manifest *previous_manifest;

            if(flags & FLAG_NO_UPGRADE)
                previous_manifest = NULL;
            else
                previous_manifest = open_provided_or_previous_manifest_file(old_manifest, coordinator_profile_path, profile, MANIFEST_SNAPSHOT_FLAG, container_filter, component_filter);

            if(previous_manifest == NULL || check_manifest(previous_manifest))
                exit_status = !migrate(manifest, previous_manifest, max_concurrent_transfers, flags, keep);
            else
                exit_status = 1;

            /* Cleanup */
            delete_manifest(previous_manifest);
        }
        else
            exit_status = 1;

        delete_manifest(manifest);

        /* Return the exit status */
        return exit_status;
    }
}
