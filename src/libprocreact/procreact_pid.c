/*
 * Copyright (c) 2016-2022 Sander van der Burg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
