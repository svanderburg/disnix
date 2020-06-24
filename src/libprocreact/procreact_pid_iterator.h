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

#ifndef __PROCREACT_PID_ITERATOR_H
#define __PROCREACT_PID_ITERATOR_H
#include "procreact_pid.h"
#include "procreact_util.h"

/** Pointer to a function that determines whether there is a next element in the collection */
typedef ProcReact_bool (*ProcReact_PidIteratorHasNext) (void *data);

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
ProcReact_bool procreact_spawn_next_pid(ProcReact_PidIterator *iterator);

/**
 * Waits for any process to complete and executes its corresponding complete callback.
 *
 * @param iterator PID iterator
 * @return TRUE if there are any running processes completed, else FALSE
 */
ProcReact_bool procreact_wait_for_process_to_complete(ProcReact_PidIterator *iterator);

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
void procreact_fork_and_wait_in_parallel_limit(ProcReact_PidIterator *iterator, const unsigned int limit);

#endif
