/*
 * Copyright (c) 2016-2020 Sander van der Burg
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

#include "procreact_future.h"
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define TRUE 1
#define FALSE 0

ProcReact_Future procreact_initialize_future(ProcReact_Type type)
{
    ProcReact_Future future;
    int pipefd[2];
    
    future.type = type;
    future.result = NULL;
    
    if(pipe(pipefd) == 0)
    {
        future.pid = fork();
        
        if(future.pid == 0)
        {
            close(pipefd[0]); /* Close read-end of pipe */
            future.fd = pipefd[1];
        }
        else
        {
            close(pipefd[1]); /* Close write-end of pipe */
            future.fd = pipefd[0];
        }
    }
    else
    {
        future.pid = -1;
        future.fd = -1;
    }
    
    return future;
}

void procreact_destroy_future(ProcReact_Future *future)
{
    close(future->fd);
}

void *procreact_future_get(ProcReact_Future *future, ProcReact_Status *status)
{
    if(future->result == NULL)
    {
        future->state = future->type.initialize();
        
        while(future->type.append(&future->type, future->state, future->fd) > 0);
        
        future->result = future->type.finalize(future->state, future->pid, status);
        
        /* Destroy the future's resources as we no longer need them */
        procreact_destroy_future(future);
    }

    return future->result;
}
