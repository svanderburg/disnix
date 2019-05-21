/*
 * Copyright (c) 2016-2019 Sander van der Burg
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

#include "procreact_future_iterator.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

ProcReact_FutureIterator procreact_initialize_future_iterator(ProcReact_FutureIteratorHasNext has_next, ProcReact_FutureIteratorNext next, ProcReact_FutureIteratorComplete complete, void *data)
{
    ProcReact_FutureIterator iterator = { has_next, next, complete, data, 0, NULL };
    return iterator;
}

void procreact_destroy_future_iterator(ProcReact_FutureIterator *iterator)
{
    free(iterator->futures);
}

int procreact_spawn_next_future(ProcReact_FutureIterator *iterator)
{
    if(iterator->has_next(iterator->data))
    {
        ProcReact_Future future = iterator->next(iterator->data);
    
        if(future.pid == -1 || future.fd == -1)
            iterator->complete(iterator->data, &future, PROCREACT_STATUS_FORK_FAIL);
        else
        {
            future.state = future.type.initialize();
            
            iterator->running_processes++;
            iterator->futures = (ProcReact_Future*)realloc(iterator->futures, iterator->running_processes * sizeof(ProcReact_Future));
            iterator->futures[iterator->running_processes - 1] = future;
        }
    
        return TRUE;
    }
    else
        return FALSE;
}

unsigned int procreact_buffer(ProcReact_FutureIterator *iterator)
{
    unsigned int i;
    
    /* Buffer the output of all processes breadth first */
    for(i = 0; i < iterator->running_processes; i++)
    {
        ProcReact_Future *future = &iterator->futures[i];
        ssize_t bytes_read = future->type.append(&future->type, future->state, future->fd);
        
        if(bytes_read <= 0)
        {
            /* If a process indicates that it's ready, finalize the buffer */
            
            ProcReact_Status status;
            future->result = future->type.finalize(future->state, future->pid, &status);
            iterator->complete(iterator->data, future, status);
            
            /* Destroy the future's resources as we no longer need them */
            procreact_destroy_future(future);
            
            /* Put future at the end of the list and decrease the size */
            iterator->futures[i] = iterator->futures[iterator->running_processes - 1];
            iterator->running_processes--;
        }
    }
    
    return iterator->running_processes;
}

void procreact_fork_in_parallel_buffer_and_wait(ProcReact_FutureIterator *iterator)
{
    /* Fork processes in parallel */
    while(procreact_spawn_next_future(iterator));
    
    /* Capture the output of each future breadth first until all processes have been terminated */
    while(procreact_buffer(iterator) > 0);
}

void procreact_fork_buffer_and_wait_in_parallel_limit(ProcReact_FutureIterator *iterator, const unsigned int limit)
{
    /* Repeat this until all processes have been spawned and finished */
    
    while(iterator->running_processes > 0 || iterator->has_next(iterator->data))
    {
        unsigned int old_running_processes;
        
        /* Fork at most the 'limit' number of processes in parallel */
        while(iterator->running_processes < limit && procreact_spawn_next_future(iterator));
        
        /* Keep capturing the output of each future breadth first, until at least one process terminates */
        old_running_processes = iterator->running_processes;
        while(procreact_buffer(iterator) == old_running_processes);
    }
}
