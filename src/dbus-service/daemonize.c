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

#include "daemonize.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define NULL_DEV_FILE "/dev/null"
#define TRUE 1
#define FALSE 0
#define BUFFER_SIZE 10

static int close_non_standard_file_descriptors(void)
{
    unsigned int i;

    struct rlimit rlim;
    int num_of_fds = getrlimit(RLIMIT_NOFILE, &rlim);

    if(num_of_fds == -1)
        return FALSE;

    for(i = 3; i < num_of_fds; i++)
        close(i);

    return TRUE;
}

static int reset_signal_handlers_to_default(void)
{
#if defined _NSIG
    unsigned int i;

    for(i = 1; i < _NSIG; i++)
    {
         if(i != SIGKILL && i != SIGSTOP)
             signal(i, SIG_DFL);
    }
#endif
    return TRUE;
}

static int clear_signal_mask(void)
{
    sigset_t set;

    return((sigemptyset(&set) == 0)
      && (sigprocmask(SIG_SETMASK, &set, NULL) == 0));
}

static int create_pid_file(const char *pid_file)
{
    pid_t my_pid = getpid();
    char my_pid_str[10];
    int fd;

    sprintf(my_pid_str, "%d", my_pid);

    if((fd = open(pid_file, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR)) == -1)
        return FALSE;

    if(write(fd, my_pid_str, strlen(my_pid_str)) == -1)
        return FALSE;

    close(fd);

    return TRUE;
}

static int attach_standard_file_descriptors_to_null(void)
{
    int null_fd_read, null_fd_write;

    return(((null_fd_read = open(NULL_DEV_FILE, O_RDONLY)) != -1)
      && (dup2(null_fd_read, STDIN_FILENO) != -1)
      && ((null_fd_write = open(NULL_DEV_FILE, O_WRONLY)) != -1)
      && (dup2(null_fd_write, STDOUT_FILENO) != -1)
      && (dup2(null_fd_write, STDERR_FILENO) != -1));
}

static void notify_parent_process(int writefd, DaemonStatus message)
{
    char byte = (char)message;
    while(write(writefd, &byte, 1) == 0);
    close(writefd);
}

static pid_t fork_daemon_process(int writefd, const char *pid_file, void *data, int (*initialize_daemon) (void *data), int (*run_main_loop) (void *data))
{
    pid_t pid = fork();

    if(pid == 0)
    {
        if(!attach_standard_file_descriptors_to_null())
        {
            notify_parent_process(writefd, STATUS_CANNOT_ATTACH_STD_FDS_TO_NULL);
            exit(STATUS_CANNOT_ATTACH_STD_FDS_TO_NULL);
        }

        umask(0);

        if(chdir("/") == -1)
        {
            notify_parent_process(writefd, STATUS_CANNOT_CHDIR);
            exit(STATUS_CANNOT_CHDIR);
        }

        if(!create_pid_file(pid_file))
        {
            notify_parent_process(writefd, STATUS_CANNOT_CREATE_PID_FILE);
            exit(STATUS_CANNOT_CREATE_PID_FILE);
        }

        if(!initialize_daemon(data))
        {
            notify_parent_process(writefd, STATUS_CANNOT_INIT_DAEMON);
            unlink(pid_file);
            exit(STATUS_CANNOT_INIT_DAEMON);
        }

        notify_parent_process(writefd, STATUS_INIT_SUCCESS);

        int exit_status = run_main_loop(data);

        if(unlink(pid_file) == 0)
            exit(STATUS_CANNOT_UNLINK_PID_FILE);

        exit(exit_status);
    }

    return pid;
}

static pid_t fork_helper_process(int pipefd[2], const char *pid_file, void *data, int (*initialize_daemon) (void *data), int (*run_main_loop) (void *data))
{
    pid_t pid = fork();

    if(pid == 0)
    {
        close(pipefd[0]); /* Close unneeded read-end */

        if(setsid() == -1)
        {
            notify_parent_process(pipefd[1], STATUS_CANNOT_SET_SID);
            exit(STATUS_CANNOT_SET_SID);
        }

        /* Fork again, so that the terminal can not be acquired again */
        if(fork_daemon_process(pipefd[1], pid_file, data, initialize_daemon, run_main_loop) == -1)
        {
            notify_parent_process(pipefd[1], STATUS_CANNOT_FORK_DAEMON_PROCESS);
            exit(STATUS_CANNOT_FORK_DAEMON_PROCESS);
        }

        exit(0); /* Exit the helper process, so that the daemon process gets adopted by PID 1 */
    }

    return pid;
}

static DaemonStatus wait_for_notification_message(int readfd)
{
    char buf[BUFFER_SIZE];
    ssize_t bytes_read = read(readfd, buf, 1);

    if(bytes_read == -1)
        return STATUS_CANNOT_READ_FROM_PIPE;
    else if(bytes_read == 0)
        return STATUS_UNKNOWN_DAEMON_ERROR;
    else
        return buf[0];
}

DaemonStatus daemonize(const char *pid_file, void *data, int (*initialize_daemon) (void *data), int (*run_main_loop) (void *data))
{
    int pipefd[2];

    if(!close_non_standard_file_descriptors())
        return STATUS_CANNOT_CLOSE_NON_STD_FDS;

    if(!reset_signal_handlers_to_default())
        return STATUS_CANNOT_RESET_SIGNAL_HANDLERS;

    if(!clear_signal_mask())
        return STATUS_CANNOT_CLEAR_SIGNAL_MASK;

    if(pipe(pipefd) == -1)
        return STATUS_CANNOT_CREATE_PIPE;
    else
    {
        if(fork_helper_process(pipefd, pid_file, data, initialize_daemon, run_main_loop) == -1)
            return STATUS_CANNOT_FORK_HELPER_PROCESS;
        else
        {
             DaemonStatus exit_status;

             close(pipefd[1]); /* Close unneeded write end */
             exit_status = wait_for_notification_message(pipefd[0]);
             close(pipefd[0]);
             return exit_status;
        }
    }
}

void print_daemon_status(DaemonStatus status, FILE *file)
{
    switch(status)
    {
        case STATUS_INIT_SUCCESS:
            fprintf(file, "Daemon initialized successfully!\n");
            break;
        case STATUS_CANNOT_ATTACH_STD_FDS_TO_NULL:
            fprintf(file, "Cannot attach standard file descriptors to " NULL_DEV_FILE " !\n");
            break;
        case STATUS_CANNOT_CHDIR:
            fprintf(file, "Cannot change current working directory to /\n");
            break;
        case STATUS_CANNOT_CREATE_PID_FILE:
            fprintf(file, "Cannot create PID file!\n");
            break;
        case STATUS_CANNOT_INIT_DAEMON:
            fprintf(file, "Cannot initialize the daemon!\n");
            break;
        case STATUS_CANNOT_UNLINK_PID_FILE:
            fprintf(file, "Cannot unlink PID file!\n");
            break;
        case STATUS_CANNOT_CLOSE_NON_STD_FDS:
            fprintf(file, "Cannot close the non standard file descriptors!\n");
            break;
        case STATUS_CANNOT_RESET_SIGNAL_HANDLERS:
            fprintf(file, "Cannot reset signal handlers!\n");
            break;
        case STATUS_CANNOT_CLEAR_SIGNAL_MASK:
            fprintf(file, "Cannot clear signal mask!\n");
            break;
        case STATUS_CANNOT_CREATE_PIPE:
            fprintf(file, "Cannot create pipe!\n");
            break;
        case STATUS_CANNOT_FORK_HELPER_PROCESS:
            fprintf(file, "Cannot fork helper process!\n");
            break;
        case STATUS_CANNOT_READ_FROM_PIPE:
            fprintf(file, "Cannot read from pipe!\n");
            break;
        case STATUS_CANNOT_SET_SID:
            fprintf(file, "Cannot set session ID!\n");
            break;
        case STATUS_CANNOT_FORK_DAEMON_PROCESS:
            fprintf(file, "Cannot fork daemon process!\n");
            break;
        case STATUS_UNKNOWN_DAEMON_ERROR:
            fprintf(file, "Unknown daemon error!\n");
            break;
    }
}
