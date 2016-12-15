#ifndef __PROCREACT_PID_ITERATOR_H
#define __PROCREACT_PID_ITERATOR_H
#include "procreact_pid.h"

/** Pointer to a function that determines whether there is a next element in the collection */
typedef int (*ProcReact_PidIteratorHasNext) (void *data);

/** Pointer to a function that spawns the next process in the collection */
typedef pid_t (*ProcReact_PidIteratorNext) (void *data);

/** Pointer to a function that gets executed when a processes finishes */
typedef void (*ProcReact_PidIteratorComplete) (void *data, pid_t pid, ProcReact_Status, int result);

typedef struct ProcReact_PidIterator ProcReact_PidIterator;

/**
 * @brief Encapsulates an iterator that can be used to spawn a collection of processes
 */
struct ProcReact_PidIterator
{
    /**
     * Function that determines whether there is a next element in the
     * collection.
     *
     * @param data Data structure used to compute the overall outcome
     * @return int TRUE if there is a next element, else FALSE
     */
    ProcReact_PidIteratorHasNext has_next;
    
    /**
     * Function that spawns the next process in the collection
     *
     * @param data Data structure used to compute the overall outcome
     * @return PID of the spawned process
     */
    ProcReact_PidIteratorNext next;
    
    /**
     * Function that retrieves the end result from the exit status
     *
     * @param pid PID of a process
     * @param wstatus Wait status of a process
     * @param status Status option that will be set to any of the status codes
     * @return The result derived from the exit status
     */
    ProcReact_RetrieveResult retrieve;
    
    /**
     * Function that gets executed when a processes finishes
     *
     * @param data Data structure used to compute the overall outcome
     * @param pid PID of the finished process
     * @param status Status option that will be set to any of the status codes
     * @param result Contains the result from the retrieval function
     */
    ProcReact_PidIteratorComplete complete;
    
    /** Arbitrary data structure used to compute the overall end result */
    void *data;
    
    /** Memorizes the amount of processes running concurrently */
    unsigned int running_processes;
};

/**
 * Creates a new PID iterator struct.
 *
 * @param has_next Function that determines whether there is a next element in the collection.
 * @param next Function that spawns the next process in the collection
 * @param retrieve Function that retrieves the end result from the exit status
 * @param complete Function that gets executed when a processes finishes
 * @param data Arbitrary data structure used to compute the overall end result
 * @return A PID iterator struct
 */
ProcReact_PidIterator procreact_initialize_pid_iterator(ProcReact_PidIteratorHasNext has_next, ProcReact_PidIteratorNext next, ProcReact_RetrieveResult retrieve, ProcReact_PidIteratorComplete complete, void *data);

/**
 * Spawns the next process in the collection
 *
 * @param iterator PID iterator
 * @return TRUE if there are more processes in the collection, FALSE if all have been spawned
 */
int procreact_spawn_next_pid(ProcReact_PidIterator *iterator);

/**
 * Waits for any process to complete and executes its corresponding complete callback.
 *
 * @param iterator PID iterator
 * @return TRUE if there are any running processes completed, else FALSE
 */
int procreact_wait_for_process_to_complete(ProcReact_PidIterator *iterator);

/**
 * Spawns all processes in a collection in parallel and waits for their
 * completion.
 *
 * @param iterator PID iterator
 */
void procreact_fork_in_parallel_and_wait(ProcReact_PidIterator *iterator);

/**
 * Spawns a specified number of processes in a collection in parallel and waits
 * for their completion.
 *
 * @param iterator PID iterator
 * @param limit Amount of processes that are allowed to run concurrently
 */
void procreact_fork_and_wait_in_parallel_limit(ProcReact_PidIterator *iterator, unsigned int limit);

#endif
