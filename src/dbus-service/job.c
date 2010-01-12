#include "job.h"

Job *job_new(Operation operation, gpointer params, GError **error)
{
    Job *job = (Job*)g_malloc(sizeof(Job));
    job->operation = operation;
    job->params = params;
    job->running = FALSE;
    return job;
}
