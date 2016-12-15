#ifndef __PROCREACT_SIGNAL_H
#define __PROCREACT_SIGNAL_H
#include "procreact_pid_iterator.h"

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

#endif
