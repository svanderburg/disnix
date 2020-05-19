#include "copy-snapshots.h"
#include <libgen.h>
#include <unistd.h>
#include <procreact_types.h>
#include <client-interface.h>
#include "state-management.h"

// TODO: see what to do if #missing_snapshots == 0

int copy_snapshots_to_sync(gchar *interface, gchar *target, gchar *container, gchar *component, int all)
{
    int exit_status = TRUE;
    char **snapshots;

    if(all)
        snapshots = statemgmt_query_all_snapshots_sync(container, component, 2);
    else
        snapshots = statemgmt_query_latest_snapshot_sync(container, component, 2);

    if(snapshots == NULL)
        exit_status = FALSE;
    else
    {
        unsigned int i;

        for(i = 0; i < g_strv_length(snapshots); i++) // We need to traverse the snapshots in the right order, one by one, to ensure that the generations are imported in the right order
        {
            char *snapshot = snapshots[i];
            char *snapshot_array[] = { snapshot, NULL };

            char **missing_snapshots = exec_print_missing_snapshots_sync(interface, target, snapshot_array, g_strv_length(snapshot_array));

            if(missing_snapshots == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(missing_snapshots) == 0) // If no snapshots need to be transferred, we still have to order them
                {
                     char **resolved_snapshots = exec_resolve_snapshots_sync(interface, target, snapshot_array, g_strv_length(snapshot_array));
                     exit_status = exec_import_remote_snapshots_sync(interface, target, container, component, resolved_snapshots, g_strv_length(resolved_snapshots));
                     procreact_free_string_array(resolved_snapshots);
                }
                else
                {
                    char **resolved_snapshots = statemgmt_resolve_snapshots_sync(missing_snapshots, 2);

                    if(resolved_snapshots == NULL)
                        exit_status = FALSE;
                    else
                    {
                        if(g_strv_length(resolved_snapshots) == 0)
                            exit_status = FALSE;
                        else
                            exit_status = exec_import_local_snapshots_sync(interface, target, container, component, resolved_snapshots, g_strv_length(resolved_snapshots));

                        procreact_free_string_array(resolved_snapshots);
                    }
                }

                procreact_free_string_array(missing_snapshots);
            }
        }

        procreact_free_string_array(snapshots);
    }

    return exit_status;
}

pid_t copy_snapshots_to(gchar *interface, gchar *target, gchar *container, gchar *component, int all)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_snapshots_to_sync(interface, target, container, component, all));

    return pid;
}

int copy_snapshots_from_sync(gchar *interface, gchar *target, gchar *container, gchar *component, int all)
{
    int exit_status = TRUE;
    char **snapshots;

    if(all)
        snapshots = exec_query_all_snapshots_sync(interface, target, container, component);
    else
        snapshots = exec_query_latest_snapshot_sync(interface, target, container, component);

    if(snapshots == NULL)
        exit_status = FALSE;
    else
    {
        unsigned int i;

        for(i = 0; i < g_strv_length(snapshots); i++) // We need to traverse the snapshots in the right order, one by one, to ensure that the generations are imported in the right order
        {
            char *snapshot = snapshots[i];
            char *snapshot_array[] = { snapshot, NULL };

            char **missing_snapshots = statemgmt_print_missing_snapshots_sync(snapshots, 2);

            if(missing_snapshots == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(missing_snapshots) == 0) // If no snapshots need to be transferred, we still have to order them
                {
                    char **resolved_snapshots = statemgmt_resolve_snapshots_sync(snapshot_array, 2);

                    if(resolved_snapshots == NULL)
                        exit_status = FALSE;
                    else
                    {
                        exit_status = statemgmt_import_snapshots_sync(container, component, resolved_snapshots, 1, 2);
                        procreact_free_string_array(resolved_snapshots);
                    }
                }
                else
                {
                    char **resolved_snapshots = exec_resolve_snapshots_sync(interface, target, missing_snapshots, g_strv_length(missing_snapshots));

                    if(resolved_snapshots == NULL)
                        exit_status = FALSE;
                    else
                    {
                        if(g_strv_length(resolved_snapshots) == 0)
                            exit_status = FALSE;
                        else
                        {
                            char **tmpdirs = exec_export_remote_snapshots_sync(interface, target, resolved_snapshots, g_strv_length(resolved_snapshots));

                            if(tmpdirs == NULL)
                                exit_status = FALSE;
                            else
                            {
                                if(g_strv_length(tmpdirs) == 0)
                                    exit_status = FALSE;
                                else
                                {
                                    char *tmpdir = tmpdirs[0];
                                    gchar *tmp_snapshot = g_strconcat(tmpdir, "/", basename(resolved_snapshots[0]), NULL);
                                    char *tmp_snapshots[] = { tmp_snapshot, NULL };

                                    exit_status = statemgmt_import_snapshots_sync(container, component, tmp_snapshots, 1, 2);

                                    rmdir(tmpdir);
                                    g_free(tmp_snapshot);
                                }

                                g_strfreev(tmpdirs);
                            }
                        }

                        procreact_free_string_array(resolved_snapshots);
                    }
                }

                procreact_free_string_array(missing_snapshots);
            }
        }

        procreact_free_string_array(snapshots);
    }

    return exit_status;
}

/*int copy_snapshots_from_sync(gchar *interface, gchar *target, gchar *container, gchar *component, int all)
{
    int exit_status = TRUE;
    char **snapshots;

    if(all)
        snapshots = exec_query_all_snapshots_sync(interface, target, container, component);
    else
        snapshots = exec_query_latest_snapshot_sync(interface, target, container, component);

    if(snapshots == NULL)
        exit_status = FALSE;
    else if(g_strv_length(snapshots) > 0)
    {
        char **missing_snapshots = statemgmt_print_missing_snapshots_sync(snapshots, 2);

        if(missing_snapshots == NULL)
            exit_status = FALSE;
        else if(g_strv_length(missing_snapshots) > 0)
        {
            char **resolved_snapshots = exec_resolve_snapshots_sync(interface, target, missing_snapshots, g_strv_length(missing_snapshots));

            if(resolved_snapshots == NULL)
                exit_status = FALSE;
            else
            {
                unsigned int i;

                for(i = 0; i < g_strv_length(resolved_snapshots); i++)
                {
                    gchar *resolved_snapshot = resolved_snapshots[i];
                    gchar *resolved_snapshot_array[] = { resolved_snapshot, NULL };
                    char **tmpdirs = exec_export_remote_snapshots_sync(interface, target, resolved_snapshot_array, g_strv_length(resolved_snapshots));

                    if(tmpdirs == NULL)
                    {
                        g_printerr("Cannot export snapshot: %s\n", resolved_snapshot);
                        break;
                    }
                    else
                    {
                        if(g_strv_length(tmpdirs) > 0)
                        {
                            char *tmpdir = tmpdirs[0];
                            gchar *tmp_snapshot = g_strconcat(tmpdir, "/", basename(resolved_snapshot), NULL);
                            char *tmp_snapshots[] = { tmp_snapshot, NULL };

                            if(!statemgmt_import_snapshots_sync(container, component, tmp_snapshots, 1, 2))
                            {
                                exit_status = FALSE;
                                break;
                            }

                            rmdir(tmpdir);
                            g_free(tmp_snapshot);
                        }

                        g_strfreev(tmpdirs);
                    }
                }

                procreact_free_string_array(resolved_snapshots);
            }

            procreact_free_string_array(missing_snapshots);
        }

        procreact_free_string_array(snapshots);
    }

    return exit_status;
}
*/

pid_t copy_snapshots_from(gchar *interface, gchar *target, gchar *container, gchar *component, int all)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_snapshots_from_sync(interface, target, container, component, all));

    return pid;
}
