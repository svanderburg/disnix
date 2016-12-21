/*
 * Copyright (c) 2016 Sander van der Burg
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

#include "procreact_pid_iterator.h"
#include <sys/wait.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

ProcReact_PidIterator procreact_initialize_pid_iterator(ProcReact_PidIteratorHasNext has_next, ProcReact_PidIteratorNext next, ProcReact_RetrieveResult retrieve, ProcReact_PidIteratorComplete complete, void *data)
{
    ProcReact_PidIterator iterator = { has_next, next, retrieve, complete, data, 0 };
    return iterator;
}

int procreact_spawn_next_pid(ProcReact_PidIterator *iterator)
{
    if(iterator->has_next(iterator->data))
    {
        pid_t pid = iterator->next(iterator->data);
        
        if(pid == -1)
            iterator->complete(iterator->data, pid, PROCREACT_STATUS_FORK_FAIL, -1);
        else
            iterator->running_processes++;
        
        return TRUE;
    }
    else
        return FALSE;
}

int procreact_wait_for_process_to_complete(ProcReact_PidIterator *iterator)
{
    if(iterator->running_processes > 0)
    {
        int wstatus, result;
        ProcReact_Status status;
        
        /* Wait for one of the processes to finish */
        pid_t pid = wait(&wstatus);
        
        if(pid > 0)
        {
            result = iterator->retrieve(pid, wstatus, &status);
            iterator->running_processes--;
        }
        else
            result = 1;
        
        iterator->complete(iterator->data, pid, status, result);
        
        return TRUE;
    }
    else
        return FALSE;
}

void procreact_fork_in_parallel_and_wait(ProcReact_PidIterator *iterator)
{
    /* Fork all processes in parallel */
    while(procreact_spawn_next_pid(iterator));
    
    /* Wait for all running processes to complete */
    while(procreact_wait_for_process_to_complete(iterator));
}

void procreact_fork_and_wait_in_parallel_limit(ProcReact_PidIterator *iterator, const unsigned int limit)
{
    /* Repeat this until all processes have been spawned and finished */
    int has_running_processes = FALSE;
    
    while(has_running_processes || iterator->has_next(iterator->data))
    {
        /* Fork at most the 'limit' number of processes in parallel */
        while(iterator->running_processes < limit && procreact_spawn_next_pid(iterator));
        
        /* Wait for one of the processes to finish */
        has_running_processes = procreact_wait_for_process_to_complete(iterator);
    }
}
