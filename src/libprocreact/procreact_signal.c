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
