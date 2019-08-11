#include "deploy.h"
#include <migrate.h>
#include "distribute.h"
#include "activate.h"
#include "locking.h"
#include "set-profiles.h"

static int distribute_closures(Manifest *manifest, const unsigned int max_concurrent_transfers)
{
    g_print("[coordinator]: Distributing intra-dependency closures...\n");
    return distribute(manifest, max_concurrent_transfers);
}

static TransitionStatus activate_new_configuration(gchar *old_manifest_file, const gchar *new_manifest, Manifest *manifest, Manifest *old_manifest, gchar *profile, const gchar *coordinator_profile_path, const unsigned int flags, void (*pre_hook) (void), void (*post_hook) (void))
{
    TransitionStatus status;

    g_print("[coordinator]: Activating new configuration...\n");

    status = activate_system(manifest, old_manifest, flags, pre_hook, post_hook);
    print_transition_status(status, old_manifest_file, new_manifest, coordinator_profile_path, profile);

    return status;
}

static int acquire_locks(Manifest *manifest, const unsigned int flags, gchar *profile, void (*pre_hook) (void), void (*post_hook) (void))
{
    if(flags & FLAG_NO_LOCK)
    {
        g_print("[coordinator]: Not acquiring any locks...\n");
        return TRUE;
    }
    else
    {
        g_print("[coordinator]: Acquiring locks...\n");
        return lock(manifest->profile_mapping_table, manifest->targets_table, profile, pre_hook, post_hook);
    }
}

static int release_locks(Manifest *manifest, const unsigned int flags, gchar *profile, void (*pre_hook) (void), void (*post_hook) (void))
{
    if(flags & FLAG_NO_LOCK)
    {
        g_print("[coordinator]: Not releasing any locks...\n");
        return TRUE;
    }
    else
    {
        g_print("[coordinator]: Releasing locks...\n");
        return unlock(manifest->profile_mapping_table, manifest->targets_table, profile, pre_hook, post_hook);
    }
}

static int migrate_data(Manifest *manifest, Manifest *old_manifest, const unsigned int max_concurrent_transfers, const unsigned int flags, const unsigned int keep)
{
    if(flags & FLAG_NO_MIGRATION)
        return TRUE;
    else
    {
        g_print("[coordinator]: Migrating data...\n");
        return migrate(manifest, old_manifest, max_concurrent_transfers, flags, keep);
    }
}

static int set_all_profiles(Manifest *manifest, const gchar *new_manifest, const gchar *coordinator_profile_path, gchar *profile)
{
    g_print("[coordinator]: Setting profiles...\n");
    return set_profiles(manifest, new_manifest, coordinator_profile_path, profile, 0);
}

DeployStatus deploy(gchar *old_manifest_file, const gchar *new_manifest_file, Manifest *manifest, Manifest *old_manifest, gchar *profile, const gchar *coordinator_profile_path, const unsigned int max_concurrent_transfers, const unsigned int keep, const unsigned int flags, void (*pre_hook) (void), void (*post_hook) (void))
{
    if(!distribute_closures(manifest, max_concurrent_transfers))
        return DEPLOY_FAIL;

    if(!acquire_locks(manifest, flags, profile, pre_hook, post_hook))
        return DEPLOY_FAIL;

    if(activate_new_configuration(old_manifest_file, new_manifest_file, manifest, old_manifest, profile, coordinator_profile_path, flags, pre_hook, post_hook) != 0)
    {
        release_locks(manifest, flags, profile, pre_hook, post_hook);
        return DEPLOY_FAIL;
    }

    if(!migrate_data(manifest, old_manifest, max_concurrent_transfers, flags, keep))
    {
        release_locks(manifest, flags, profile, pre_hook, post_hook);
        return DEPLOY_STATE_FAIL;
    }

    if(!set_all_profiles(manifest, new_manifest_file, coordinator_profile_path, profile))
    {
        release_locks(manifest, flags, profile, pre_hook, post_hook);
        return DEPLOY_FAIL;
    }

    if(!release_locks(manifest, flags, profile, pre_hook, post_hook))
        return DEPLOY_FAIL;

    return DEPLOY_OK;
}
