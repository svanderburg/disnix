/*
 * Copyright (c) 2016-2019 Sander van der Burg
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

#ifndef __PROCREACT_FUTURE_ITERATOR_H
#define __PROCREACT_FUTURE_ITERATOR_H
#include "procreact_pid.h"
#include "procreact_future.h"

/** Pointer to a function that determines whether there is a next element in the collection */
typedef int (*ProcReact_FutureIteratorHasNext) (void *data);

/** Pointer to a function that spawns the next future in the collection */
typedef ProcReact_Future (*ProcReact_FutureIteratorNext) (void *data);

/** Pointer to a function that gets executed when a processes finishes */
typedef void (*ProcReact_FutureIteratorComplete) (void *data, ProcReact_Future *future, ProcReact_Status status);

typedef struct ProcReact_FutureIterator ProcReact_FutureIterator;

/**
 * @brief Encapsulates an iterator that can be used to spawn a collection of futures
 */
struct ProcReact_FutureIterator
{
    /**
     * Function that determines whether there is a next element in the
     * collection.
     *
     * @param data Data structure used to compute the overall outcome
     * @return int TRUE if there is a next element, else FALSE
     */
    ProcReact_FutureIteratorHasNext has_next;

    /**
     * Function that spawns the next future in the collection.
     *
     * @param data Data structure used to compute the overall outcome
     * @return A future instance
     */
    ProcReact_FutureIteratorNext next;

    /**
     * Function that gets invoked if a process completes.
     *
     * @param data Data structure used to compute the overall outcome
     * @param future Future struct instance of the process that has completed
     * @param status Status option that will be set to any of the status codes
     */
    ProcReact_FutureIteratorComplete complete;

    /** Arbitrary data structure used to compute the overall end result */
    void *data;

    /** Memorizes the amount of processes running concurrently */
    unsigned int running_processes;

    /** Memorizes the future instances of the process that are being executed */
    ProcReact_Future *futures;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a new future iterator struct.
 *
 * @param has_next Function that determines whether there is a next element in the collection
 * @param next Function that spawns the next future in the collection
 * @param complete Function that gets invoked if a process completes
 * @param data Arbitrary data structure used to compute the overall end result
 * @return Future iterator struct
 */
ProcReact_FutureIterator procreact_initialize_future_iterator(ProcReact_FutureIteratorHasNext has_next, ProcReact_FutureIteratorNext next, ProcReact_FutureIteratorComplete complete, void *data);

/**
 * Clears all resources allocated with a future iterator.
 *
 * @param iterator Future iterator
 */
void procreact_destroy_future_iterator(ProcReact_FutureIterator *iterator);

/**
 * Spawns the next process in the collection
 *
 * @param iterator Future iterator
 * @return TRUE if there are more processes in the collection, FALSE if all have been spawned
 */
int procreact_spawn_next_future(ProcReact_FutureIterator *iterator);

/**
 * Reads the data from the read-end of each pipe of a running process and
 * buffers their state.
 *
 * @param iterator Future iterator
 * @return The amount of running processes
 */
unsigned int procreact_buffer(ProcReact_FutureIterator *iterator);

/**
 * Spawns all futures in a collection in parallel, gathers their outputs and
 * waits for their completion.
 *
 * @param iterator Future iterator
 */
void procreact_fork_in_parallel_buffer_and_wait(ProcReact_FutureIterator *iterator);

/**
 * Spawns a specified number of futures in a collection in parallel, gathers
 * their outputs and waits for their completion.
 *
 * @param iterator Future iterator
 * @param limit Amount of processes that are allowed to run concurrently
 */
void procreact_fork_buffer_and_wait_in_parallel_limit(ProcReact_FutureIterator *iterator, const unsigned int limit);

#ifdef __cplusplus
}
#endif

#endif
