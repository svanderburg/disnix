#include "job.h"

Job *new_job(Operation operation, gpointer params, GError **error)
{
    Job *job = (Job*)g_malloc(sizeof(Job));
    job->operation = operation;
    job->params = params;
    job->running = FALSE;
    return job;
}

void delete_job(Job *job)
{
    g_free(job);
}
