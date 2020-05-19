#include "copy-closure.h"
#include <package-management.h>
#include <client-interface.h>
#include <procreact_types.h>

int copy_closure_to_sync(gchar *interface, gchar *target, gchar *tmpdir, gchar **derivation)
{
    int exit_status = TRUE;
    char **requisites = pkgmgmt_query_requisites_sync(derivation, 2);

    if(requisites == NULL)
        exit_status = FALSE;
    else
    {
        if(g_strv_length(requisites) > 0)
        {
            char **invalid_paths = exec_print_invalid_sync(interface, target, requisites, g_strv_length(requisites));

            if(invalid_paths == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(invalid_paths) > 0)
                {
                    char *tempfile = pkgmgmt_export_closure_sync(tmpdir, invalid_paths, 2);

                    if(tempfile == NULL)
                        exit_status = FALSE;
                    else
                    {
                        exit_status = exec_import_local_closure_sync(interface, target, tempfile);
                        unlink(tempfile);
                        g_free(tempfile);
                    }
                }

                procreact_free_string_array(invalid_paths);
            }
        }

        procreact_free_string_array(requisites);
    }

    return exit_status;
}

pid_t copy_closure_to(gchar *interface, gchar *target, gchar *tmpdir, gchar **derivation)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_closure_to_sync(interface, target, tmpdir, derivation));

    return pid;
}

int copy_closure_from_sync(gchar *interface, gchar *target, gchar **derivation)
{
    int exit_status = TRUE;
    char **requisites = exec_query_requisites_sync(interface, target, derivation, g_strv_length(derivation));

    if(requisites == NULL)
        exit_status = FALSE;
    else
    {
        if(g_strv_length(requisites) > 0)
        {
            char **invalid_paths = pkgmgmt_print_invalid_packages_sync(requisites, 2);

            if(invalid_paths == NULL)
                exit_status = FALSE;
            else
            {
                if(g_strv_length(invalid_paths) > 0)
                {
                    char *tempfile = exec_export_remote_closure_sync(interface, target, invalid_paths, g_strv_length(requisites));

                    if(tempfile == NULL)
                        exit_status = FALSE;
                    else
                    {
                        exit_status = pkgmgmt_import_closure_sync(tempfile, 1, 2);
                        unlink(tempfile);
                        free(tempfile);
                    }
                }

                procreact_free_string_array(invalid_paths);
            }
        }

        procreact_free_string_array(requisites);
    }

    return exit_status;
}

pid_t copy_closure_from(gchar *interface, gchar *target, gchar **derivation)
{
    pid_t pid = fork();

    if(pid == 0)
        _exit(!copy_closure_from_sync(interface, target, derivation));

    return pid;
}
