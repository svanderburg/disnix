#include "profilelocking.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <state-management.h>
#include "profilemanifest.h"

static int lock_or_unlock_services(int log_fd, GPtrArray *profile_manifest_array, gchar *action, pid_t (*notify_function) (gchar *type, gchar *container, gchar *component, int stdout, int stderr))
{
    unsigned int i;
    int exit_status = TRUE;

    /* Notify all services for a lock or unlock */
    for(i = 0; i < profile_manifest_array->len; i++)
    {
        ProfileManifestEntry *entry = g_ptr_array_index(profile_manifest_array, i);
        pid_t pid;
        ProcReact_Status status;
        int result;

        dprintf(log_fd, "Notifying %s on %s: of type: %s in container: %s\n", action, entry->service, entry->type, entry->container);
        pid = notify_function(entry->type, entry->container, entry->service, log_fd, log_fd);
        result = procreact_wait_for_boolean(pid, &status);

        if(status != PROCREACT_STATUS_OK || !result)
        {
            dprintf(log_fd, "Cannot %s service!\n", action);
            exit_status = FALSE;
        }
    }

    return exit_status;
}

static int unlock_services(int log_fd, GPtrArray *profile_manifest_array)
{
    return lock_or_unlock_services(log_fd, profile_manifest_array, "unlock", statemgmt_unlock_component);
}

static int lock_services(int log_fd, GPtrArray *profile_manifest_array)
{
    return lock_or_unlock_services(log_fd, profile_manifest_array, "lock", statemgmt_lock_component);
}

static gchar *create_lock_filename(gchar *tmpdir, gchar *profile)
{
    return g_strconcat(tmpdir, "/disnix-", profile, ".lock", NULL);
}

static int lock_profile(int log_fd, gchar *tmpdir, gchar *profile)
{
    int fd, status;
    gchar *lock_filename = create_lock_filename(tmpdir, profile);

    /* If no lock exists, try to create one */
    if((fd = open(lock_filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
    {
        dprintf(log_fd, "Cannot exclusively open the lock file!\n");
        status = FALSE;
    }
    else
    {
        close(fd);
        status = TRUE;
    }

    g_free(lock_filename);
    return status;
}

static int unlock_profile(int log_fd, gchar *tmpdir, gchar *profile)
{
    gchar *lock_filename = create_lock_filename(tmpdir, profile);
    int status;

    if(unlink(lock_filename) == -1)
    {
        dprintf(log_fd, "There is no lock file!\n");
        status = FALSE;
    }
    else
        status = TRUE;

    /* Cleanup */
    g_free(lock_filename);

    return status;
}

int acquire_locks(int log_fd, gchar *tmpdir, GPtrArray *profile_manifest_array, gchar *profile)
{
    if(lock_services(log_fd, profile_manifest_array)) /* Attempt to acquire locks from the services */
        return lock_profile(log_fd, tmpdir, profile); /* Finally, lock the profile */
    else
    {
        unlock_services(log_fd, profile_manifest_array);
        return FALSE;
    }
}

int release_locks(int log_fd, gchar *tmpdir, GPtrArray *profile_manifest_array, gchar *profile)
{
    int status = TRUE;

    if(profile_manifest_array == NULL)
    {
        dprintf(log_fd, "Corrupt profile manifest: a service or type is missing!\n");
        status = FALSE;
    }
    else
    {
        if(!unlock_services(log_fd, profile_manifest_array))
        {
            dprintf(log_fd, "Failed to send unlock notification to old services!\n");
            status = FALSE;
        }
    }

    if(!unlock_profile(log_fd, tmpdir, profile))
        status = FALSE; /* There was no lock -> fail */

    return status;
}
