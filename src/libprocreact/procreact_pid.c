#include "procreact_pid.h"
#include <sys/types.h>
#include <sys/wait.h>

int procreact_retrieve_exit_status(pid_t pid, int wstatus, ProcReact_Status *status)
{
    if(pid == -1)
    {
        *status = PROCREACT_STATUS_WAIT_FAIL;
        return -1;
    }
    else if(WIFEXITED(wstatus))
    {
        *status = PROCREACT_STATUS_OK;
        return WEXITSTATUS(wstatus);
    }
    else
    {
        *status = PROCREACT_STATUS_ABNORMAL_TERMINATION;
        return -1;
    }
}

int procreact_retrieve_boolean(pid_t pid, int wstatus, ProcReact_Status *status)
{
    return (procreact_retrieve_exit_status(pid, wstatus, status) == 0);
}

int procreact_wait_and_retrieve(pid_t pid, ProcReact_RetrieveResult retrieve, ProcReact_Status *status)
{
    if(pid == -1)
    {
        *status = PROCREACT_STATUS_FORK_FAIL;
        return -1;
    }
    else
    {
        int wstatus;
    
        if(waitpid(pid, &wstatus, 0) == -1)
        {
            *status = PROCREACT_STATUS_WAIT_FAIL;
            return -1;
        }
        else
            return retrieve(pid, wstatus, status);
    }
}

int procreact_wait_for_exit_status(pid_t pid, ProcReact_Status *status)
{
    return procreact_wait_and_retrieve(pid, procreact_retrieve_exit_status, status);
}

int procreact_wait_for_boolean(pid_t pid, ProcReact_Status *status)
{
    return procreact_wait_and_retrieve(pid, procreact_retrieve_boolean, status);
}
