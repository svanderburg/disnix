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

#include "procreact_signal.h"
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

static volatile int empty_queue = FALSE;

static void handle_sigchld(int sig)
{
    empty_queue = TRUE;
}

int procreact_register_signal_handler(void)
{
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    return sigaction(SIGCHLD, &sa, 0);
}

void procreact_complete_all_finished_processes(ProcReact_PidIterator *iterator)
{
    if(empty_queue)
    {
        empty_queue = FALSE;

        if(iterator->running_processes > 0)
        {
            int wstatus, result;
            ProcReact_Status status;
            pid_t pid;

            /* Complete all finished processes */

            while((pid = waitpid(-1, &wstatus, WNOHANG)) > 0)
            {
                result = iterator->retrieve(pid, wstatus, &status);
                iterator->running_processes--;
                iterator->complete(iterator->data, pid, status, result);
            }
        }
    }
}
