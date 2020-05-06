/*
 * Copyright 2020 Sander van der Burg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __DAEMONIZE_H
#define __DAEMONIZE_H
#include <stdio.h>

typedef enum
{
    STATUS_INIT_SUCCESS                  = 0x0,
    STATUS_CANNOT_ATTACH_STD_FDS_TO_NULL = 0x1,
    STATUS_CANNOT_CHDIR                  = 0x2,
    STATUS_CANNOT_CREATE_PID_FILE        = 0x3,
    STATUS_CANNOT_INIT_DAEMON            = 0x4,
    STATUS_CANNOT_UNLINK_PID_FILE        = 0x5,
    STATUS_CANNOT_CLOSE_NON_STD_FDS      = 0x6,
    STATUS_CANNOT_RESET_SIGNAL_HANDLERS  = 0x7,
    STATUS_CANNOT_CLEAR_SIGNAL_MASK      = 0x8,
    STATUS_CANNOT_CREATE_PIPE            = 0x9,
    STATUS_CANNOT_FORK_HELPER_PROCESS    = 0xa,
    STATUS_CANNOT_READ_FROM_PIPE         = 0xb,
    STATUS_CANNOT_SET_SID                = 0xc,
    STATUS_CANNOT_FORK_DAEMON_PROCESS    = 0xd,
    STATUS_UNKNOWN_DAEMON_ERROR          = 0xe
}
DaemonStatus;

DaemonStatus daemonize(const char *pid_file, void *data, int (*initialize_daemon) (void *data), int (*run_main_loop) (void *data));

void print_daemon_status(DaemonStatus status, FILE *file);

#endif
