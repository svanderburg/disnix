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

#ifndef __PROCREACT_FUTURE_H
#define __PROCREACT_FUTURE_H
#include <unistd.h>
#include "procreact_pid.h"
#include "procreact_types.h"

/**
 * @brief Encapsulates a process invocation with a pipe to retrieve its output
 */
typedef struct
{
    /** PID of the process that gets executed */
    pid_t pid;
    /** Read-end of the pipe to the child process */
    int fd;
    /** Type where the read data will be converted to */
    ProcReact_Type type;
    /** Pointer to the end result or NULL if it has not yet been computed */
    void *result;
    /** Pointer to a data structure maintaining the state whill the process is being executed */
    void *state;
}
ProcReact_Future;

/**
 * Initializes a future.
 *
 * @param type Type where the read data will be converted to
 * @return A future struct
 */
ProcReact_Future procreact_initialize_future(ProcReact_Type type);

/**
 * Frees all the resources of a future.
 *
 * @param future A future struct
 */
void procreact_destroy_future(ProcReact_Future *future);

/**
 * Waits for a process to finish execution and returns the corresponding result.
 *
 * @param future A future struct
 * @param status Status option that will be set to any of the status codes
 * @return Pointer to a data struct conforming to the requested type
 */
void *procreact_future_get(ProcReact_Future *future, ProcReact_Status *status);

#endif
