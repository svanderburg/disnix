#include "deploy.h"
#include <migrate.h>
#include "distribute.h"
#include "activate.h"
#include "locking.h"
#include "profiles.h"

static int distribute_closures(Manifest *manifest, const unsigned int max_concurrent_transfers)
{
    g_print("[coordinator]: Distributing intra-dependency closures...\n");
    return distribute(manifest, max_concurrent_transfers);
}

static TransitionStatus activate_new_configuration(gchar *old_manifest_file, const gchar *new_manifest, Manifest *manifest, Manifest *old_manifest, gchar *profile, const gchar *coordinator_profile_path, const unsigned int flags)
{
    GPtrArray *old_activation_array;

    g_print("[coordinator]: Activating new configuration...\n");

    if(old_manifest == NULL)
        old_activation_array = NULL;
    else
        old_activation_array = old_manifest->activation_array;

    return activate_system(old_manifest_file, new_manifest, manifest, old_activation_array, coordinator_profile_path, profile, flags);
}

static int acquire_locks(Manifest *manifest, const unsigned int flags, gchar *profile)
{
    if(flags & FLAG_NO_LOCK)
    {
        g_print("[coordinator]: Not acquiring any locks...\n");
        return TRUE;
    }
    else
    {
        g_print("[coordinator]: Acquiring locks...\n");
        return lock(manifest->distribution_array, manifest->target_array, profile);
    }
}

static int release_locks(Manifest *manifest, const unsigned int flags, gchar *profile)
{
    if(flags & FLAG_NO_LOCK)
    {
        g_print("[coordinator]: Not releasing any locks...\n");
        return TRUE;
    }
    else
    {
        g_print("[coordinator]: Releasing locks...\n");
        return unlock(manifest->distribution_array, manifest->target_array, profile);
    }
}

static int migrate_data(Manifest *manifest, Manifest *old_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const unsigned int keep)
{
    if(flags & FLAG_NO_MIGRATION)
        return TRUE;
    else
    {
        GPtrArray *old_snapshots_array;

        if(old_manifest == NULL)
            old_snapshots_array = NULL;
        else
            old_snapshots_array = old_manifest->snapshots_array;

        g_print("[coordinator]: Migrating data...\n");
        return migrate(manifest, old_snapshots_array, max_concurrent_transfers, flags, keep);
    }
}

static int set_all_profiles(Manifest *manifest, const gchar *new_manifest, const gchar *coordinator_profile_path, gchar *profile)
{
    g_print("[coordinator]: Setting profiles...\n");
    return set_profiles(manifest, new_manifest, coordinator_profile_path, profile, FALSE, FALSE);
}

DeployStatus deploy(gchar *old_manifest_file, const gchar *new_manifest, Manifest *manifest, Manifest *old_manifest, gchar *profile, const gchar *coordinator_profile_path, const unsigned int max_concurrent_transfers, const unsigned int keep, const unsigned int flags)
{
    if(!distribute_closures(manifest, max_concurrent_transfers))
        return DEPLOY_FAIL;

    if(!acquire_locks(manifest, flags, profile))
        return DEPLOY_FAIL;

    if(activate_new_configuration(old_manifest_file, new_manifest, manifest, old_manifest, profile, coordinator_profile_path, flags) != 0)
    {
        release_locks(manifest, flags, profile);
        return DEPLOY_FAIL;
    }

    if(!migrate_data(manifest, old_manifest, max_concurrent_transfers, flags, keep))
    {
        release_locks(manifest, flags, profile);
        return DEPLOY_STATE_FAIL;
    }

    if(!set_all_profiles(manifest, new_manifest, coordinator_profile_path, profile))
    {
        release_locks(manifest, flags, profile);
        return DEPLOY_FAIL;
    }

    if(!release_locks(manifest, flags, profile))
        return DEPLOY_FAIL;

    return DEPLOY_OK;
}
