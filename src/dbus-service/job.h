#ifndef __JOB_H
#define __JOB_H
#include <glib.h>

typedef enum
{
    OP_IMPORT,
    OP_EXPORT,
    OP_PRINT_INVALID,
    OP_REALISE,
    OP_SET,
    OP_QUERY_INSTALLED,
    OP_QUERY_REQUISITES,
    OP_COLLECT_GARBAGE,
    OP_ACTIVATE,
    OP_DEACTIVATE,
}
Operation;

typedef struct
{
    Operation operation;
    gpointer params;
    GError **error;
    gboolean running;
}
Job;

Job *new_job(Operation operation, gpointer params, GError **error);

void delete_job(Job *job);

#endif
