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
#include <pwd.h>
#include <unistd.h>
#include <distributionmapping.h>
#include <manifest.h>
#include <targets.h>
#include <client-interface.h>
#include <procreact_pid_iterator.h>

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

typedef struct
{
    unsigned int index;
    unsigned int length;
    const GPtrArray *distribution_array;
    const GPtrArray *target_array;
    gchar *profile;
    int success;
    GPtrArray *lock_array;
}
DistributionIteratorData;

static int has_next_distribution_item(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    return distribution_iterator_data->index < distribution_iterator_data->length;
}

static pid_t next_unlock_process(void *data)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    pid_t pid;
    DistributionItem *item = g_ptr_array_index(distribution_iterator_data->distribution_array, distribution_iterator_data->index);
    Target *target = find_target(distribution_iterator_data->target_array, item->target);
    
    g_print("[target: %s]: Releasing a lock!\n", item->target);
    pid = exec_unlock(target->client_interface, item->target, distribution_iterator_data->profile);
    distribution_iterator_data->index++;
    return pid;
}

static void complete_unlock_process(void *data, pid_t pid, ProcReact_Status status, int result)
{
    DistributionIteratorData *distribution_iterator_data = (DistributionIteratorData*)data;
    
    if(status != PROCREACT_STATUS_OK || !result)
        distribution_iterator_data->success = FALSE;
}

static int unlock(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    DistributionIteratorData data = { 0, distribution_array->len, distribution_array, target_array, profile, TRUE };
    ProcReact_PidIterator iterator = procreact_initialize_pid_iterator(has_next_distribution_item, next_unlock_process, procreact_retrieve_boolean, complete_unlock_process, &data);
    procreact_fork_in_parallel_and_wait(&iterator);
    
    return (!data.success);
}

static int lock(const GPtrArray *distribution_array, const GPtrArray *target_array, gchar *profile)
{
    unsigned int i;
    GPtrArray *try_array = g_ptr_array_new();
    GPtrArray *lock_array = g_ptr_array_new();
    int exit_status = 0;
    int status;
    
    /* For each machine acquire a lock */
    for(i = 0; i < distribution_array->len; i++)
    {
        DistributionItem *item = g_ptr_array_index(distribution_array, i);
        Target *target = find_target(target_array, item->target);
        
        g_print("[target: %s]: Acquiring a lock\n", item->target);
        status = exec_lock(target->client_interface, item->target, profile);
        
        /* If a process fails, change the exit status */
        if(status == -1)
        {
            g_printerr("[target: %s]: Error with forking lock process!\n", item->target);
            exit_status = -1;
        }
        else
            g_ptr_array_add(try_array, item);
    }
    
    /* Wait until every lock is acquired */
    for(i = 0; i < try_array->len; i++)
    {
        status = wait_to_finish(0);
        
        /* If a process fails, change the exit status */
        if(status != 0)
        {
            g_printerr("Failed to acquire a lock!\n");
            exit_status = status;
        }
        else if(interrupted)
        {
            g_printerr("[coordinator]: The lock phase has been interrupted!\n");
            exit_status = 1;
        }
    }
    
    /* If a lock fails then unlock every machine that is locked */
    if(!exit_status)
        unlock(lock_array, target_array, profile);
    
    /* Cleanup */
    g_ptr_array_free(try_array, TRUE);
    g_ptr_array_free(lock_array, TRUE);
    
    /* Return exit status, which is 0 if everything succeeds */
    return exit_status;
}

int lock_or_unlock(const int do_lock, const gchar *manifest_file, const gchar *coordinator_profile_path, gchar *profile)
{
    GPtrArray *distribution_array;
    GPtrArray *target_array;
    int exit_status;
    
    if(manifest_file == NULL)
    {
        /* Get current username */
        char *username = (getpwuid(geteuid()))->pw_name;
        
        /* If no manifest file has been provided, try opening the last deployed one */
        gchar *old_manifest_file = determine_previous_manifest_file(coordinator_profile_path, username, profile);
        
        if(old_manifest_file == NULL)
        {
            g_printerr("[coordinator]: No previous manifest file exists, so no locking operations will be executed!\n");
            return 0;
        }
        else
        {
            distribution_array = generate_distribution_array(old_manifest_file);
            target_array = generate_target_array(old_manifest_file);
            g_free(old_manifest_file);
        }
    }
    else
    {
        /* Open the provided manifest */
        distribution_array = generate_distribution_array(manifest_file);
        target_array = generate_target_array(manifest_file);
    }
    
    if(distribution_array == NULL || target_array == NULL)
    {
        g_printerr("ERROR: Cannot open manifest file!\n");
        exit_status = 1;
    }
    else
    {
        /* Override SIGINT's behaviour to allow stuff to be rollbacked in case of an interruption */
        set_flag_on_interrupt();
        
        /* Do the locking */
        if(do_lock)
            exit_status = lock(distribution_array, target_array, profile);
        else
            exit_status = unlock(distribution_array, target_array, profile);
    }
    
    /* Cleanup */
    delete_target_array(target_array);
    delete_distribution_array(distribution_array);

    /* Return exit status */
    return exit_status;
}
