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

/**
 * @file
 * @brief PID management module
 * @defgroup PID
 * @{
 */

#ifndef __PROCREACT_PID_H
#define __PROCREACT_PID_H
#include <unistd.h>
#include "procreact_util.h"

/**
 * @brief An enumeration of possible outcomes after executing a process
 */
typedef enum
{
    /** The process was invoked successfully */
    PROCREACT_STATUS_OK,
    /** The fork() system call failed */
    PROCREACT_STATUS_FORK_FAIL,
    /** The wait() system call failed */
    PROCREACT_STATUS_WAIT_FAIL,
    /** The process was terminated abnormally */
    PROCREACT_STATUS_ABNORMAL_TERMINATION
}
ProcReact_Status;

/**
 * @brief Pointer to a function that retrieves the outcome by looking at the exit status
 */
typedef int (*ProcReact_RetrieveResult) (pid_t pid, int wstatus, ProcReact_Status *status);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Retrieves the exit status of a finished process
 *
 * @param pid PID of a process
 * @param wstatus Wait status of a process
 * @param status Status option that will be set to any of the status codes
 * @return The exit status
 */
int procreact_retrieve_exit_status(pid_t pid, int wstatus, ProcReact_Status *status);

/**
 * Retrieves a boolean status of a finished process
 * @param pid PID of a process
 * @param wstatus Wait status of a process
 * @param status Status option that will be set to any of the status codes
 * @return TRUE if the exit status is 0, else FALSE
 */
ProcReact_bool procreact_retrieve_boolean(pid_t pid, int wstatus, ProcReact_Status *status);

/**
 * Waits for a process to complete and retrieves the end result by invoking a
 * given retrieval function.
 *
 * @param pid PID of a process
 * @param retrieve A function that retrieves the result from the exit status
 * @param status Status option that will be set to any of the status codes
 * @return The return value of the retrieval function
 */
int procreact_wait_and_retrieve(pid_t pid, ProcReact_RetrieveResult retrieve, ProcReact_Status *status);

/**
 * Waits for a process to complete and returns the exit status.
 *
 * @param pid PID of a process
 * @param status Status option that will be set to any of the status codes
 * @return The exit status
 */
int procreact_wait_for_exit_status(pid_t pid, ProcReact_Status *status);

/**
 * Waits for a process to complete and returns a boolean status.
 *
 * @param pid PID of a process
 * @param status Status option that will be set to any of the status codes
 * @return TRUE if the exit status is 0, else FALSE
 */
int procreact_wait_for_boolean(pid_t pid, ProcReact_Status *status);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @}
 */
