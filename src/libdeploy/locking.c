/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2018  Sander van der Burg
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
#include <profilemapping-iterator.h>
#include <manifest.h>
#include <targetstable.h>
#include <client-interface.h>

extern volatile int interrupted;

/* Unlock infrastructure */

static pid_t unlock_distribution_item(void *data, xmlChar *profile_name, gchar *target_name, Target *target)
{
    char *profile = (char*)data;
    g_print("[target: %s]: Releasing a lock on profile: %s\n", target_name, profile_name);
    return exec_unlock((char*)target->client_interface, (char*)target_name, profile);
}

static void complete_unlock_distribution_item(void *data, xmlChar *profile_name, gchar *target_name, ProcReact_Status status, int result)
{
    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot unlock profile: %s\n", target_name, profile_name);
}

int unlock(GHashTable *profile_mapping_table, GHashTable *targets_table, gchar *profile, void (*pre_hook) (void), void (*post_hook) (void))
{
    int success;
    ProcReact_PidIterator iterator = create_distribution_iterator(profile_mapping_table, targets_table, unlock_distribution_item, complete_unlock_distribution_item, profile);

    if(pre_hook != NULL) /* Execute hook before the unlock operations are executed */
        pre_hook();

    procreact_fork_in_parallel_and_wait(&iterator);

    if(post_hook != NULL) /* Execute hook after the unlock operations have been completed */
        post_hook();

    success = distribution_iterator_has_succeeded(&iterator);

    destroy_distribution_iterator(&iterator);

    return success;
}

/* Lock infrastructure */

typedef struct
{
    gchar *profile;
    GHashTable *lock_table;
}
LockData;

static pid_t lock_distribution_item(void *data, xmlChar *profile_name, gchar *target_name, Target *target)
{
    LockData *lock_data = (LockData*)data;
    g_print("[target: %s]: Acquiring a lock on profile: %s\n", target_name, profile_name);
    return exec_lock((char*)target->client_interface, (char*)target_name, lock_data->profile);
}

static void complete_lock_distribution_item(void *data, xmlChar *profile_name, gchar *target_name, ProcReact_Status status, int result)
{
    LockData *lock_data = (LockData*)data;

    if(status != PROCREACT_STATUS_OK || !result)
        g_printerr("[target: %s]: Cannot lock profile: %s\n", target_name, profile_name);
    else
        g_hash_table_insert(lock_data->lock_table, target_name, profile_name);
}

int lock(GHashTable *profile_mapping_table, GHashTable *targets_table, gchar *profile, void (*pre_hook) (void), void (*post_hook) (void))
{
    GHashTable *lock_table = g_hash_table_new(g_str_hash, g_str_equal);
    int success;
    LockData data = { profile, lock_table };
    ProcReact_PidIterator iterator = create_distribution_iterator(profile_mapping_table, targets_table, lock_distribution_item, complete_lock_distribution_item, &data);

    if(pre_hook != NULL) /* Execute hook before the lock operations are executed */
        pre_hook();

    procreact_fork_in_parallel_and_wait(&iterator);

    if(post_hook != NULL) /* Execute hook after the lock operations have been completed */
        post_hook();

    success = distribution_iterator_has_succeeded(&iterator);

    if(interrupted)
    {
        g_printerr("[coordinator]: The lock phase has been interrupted, unlocking all targets again...\n");
        success = FALSE;
    }

    if(!success)
        unlock(lock_table, targets_table, profile, pre_hook, post_hook); /* If the locking has failed, try to unlock everything again */

    /* Cleanup */
    g_hash_table_destroy(lock_table);
    destroy_distribution_iterator(&iterator);

    return success;
}
