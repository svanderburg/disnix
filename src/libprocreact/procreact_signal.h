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
 * @brief Signal utility module
 * @defgroup Signal
 * @{
 */

#ifndef __PROCREACT_SIGNAL_H
#define __PROCREACT_SIGNAL_H
#include "procreact_pid_iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configures a SIGCHLD handler that indicates that processes have finished
 * whose results must be gathered.
 *
 * @return Same return value as sigaction()
 */
int procreact_register_signal_handler(void);

/**
 * Waits for all completed processes to finish (if any have been registered as
 * such) and gathers their results.
 *
 * @param iterator PID iterator
 */
void procreact_complete_all_finished_processes(ProcReact_PidIterator *iterator);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @}
 */
