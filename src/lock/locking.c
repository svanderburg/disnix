/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "locking.h"
#include <sys/types.h>
#include <unistd.h>
#include <distributionmapping.h>
#include <manifest.h>
#include <targets.h>
#include <client-interface.h>

volatile int interrupted;

static void handle_sigint(int signum)
{
    interrupted = TRUE;
}

static void set_flag_on_interrupt(void)
{
    struct sigaction act;
    act.sa_handler = handle_sigint;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGINT);
    
    sigaction(SIGINT, &act, NULL);
}

static pid_t unlock_distribution_item(void *data, DistributionItem *item, Target *target)
{
    char *profile = (char*)data;
    g_print("[target: %s]: Releasing a lock on profile: %s\n", item->target, item->profile);
    return exec_unlock(target->client_interface, item->target, profile);
}

static void complete_unlock_distribution_item(void *data, DistributionItem *item, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot unlock profile: %s\n", item->target, item->profile);
}

static int unlock(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    int success;
    ProcReact_PidIterator iterator = create_distribution_iterator(distribution_array, target_array, unlock_distribution_item, complete_unlock_distribution_item, profile);
    procreact_fork_in_parallel_and_wait(&iterator);
    success = distribution_iterator_has_succeeded(&iterator);
    
    destroy_distribution_iterator(&iterator);
    
    return success;
}

typedef struct
{
    gchar *profile;
    GPtrArray *lock_array;
}
LockData;

static pid_t lock_distribution_item(void *data, DistributionItem *item, Target *target)
{
    LockData *lock_data = (LockData*)data;
    g_print("[target: %s]: Acquiring a lock on profile: %s\n", item->target, item->profile);
    return exec_lock(target->client_interface, item->target, lock_data->profile);
}

static void complete_lock_distribution_item(void *data, DistributionItem *item, ProcReact_Status status, int result)
{
    LockData *lock_data = (LockData*)data;
    
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot lock profile: %s\n", item->target, item->profile);
    else
        g_ptr_array_add(lock_data->lock_array, item);
}

static int lock(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    GPtrArray *lock_array = g_ptr_array_new();
    int success;
    LockData data = { profile, lock_array };
    ProcReact_PidIterator iterator = create_distribution_iterator(distribution_array, target_array, lock_distribution_item, complete_lock_distribution_item, &data);
    procreact_fork_in_parallel_and_wait(&iterator);
    success = distribution_iterator_has_succeeded(&iterator);
    
    if(interrupted)
    {
        g_printerr("[coordinator]: The lock phase has been interrupted, unlocking all targets again...\n");
        success = FALSE;
    }
    
    if(!success)
        unlock(lock_array, target_array, profile); /* If the locking has failed, try to unlock everything again */
    
    /* Cleanup */
    g_ptr_array_free(lock_array, TRUE);
    destroy_distribution_iterator(&iterator);
    
    return success;
}

int lock_or_unlock(const int do_lock, const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile)
{
    Manifest *manifest = open_provided_or_previous_manifest_file(manifest_file, coordinator_profile_path, profile, MANIFEST_DISTRIBUTION_FLAG, NULL, NULL);

    if(manifest == NULL)
    {
        g_printerr("Cannot open any manifest file!\n");
        g_printerr("Please provide a valid manifest as command-line parameter!\n");
        return 1;
    }
    else
    {
        int exit_status;

        /* Override SIGINT's behaviour to allow stuff to be rollbacked in case of an interruption */
        set_flag_on_interrupt();
        
        /* Do the locking */
        if(do_lock)
            exit_status = !lock(manifest->distribution_array, manifest->target_array, profile);
        else
            exit_status = !unlock(manifest->distribution_array, manifest->target_array, profile);
        
        /* Cleanup */
        delete_manifest(manifest);

        /* Return exit status */
        return exit_status;
    }
}
