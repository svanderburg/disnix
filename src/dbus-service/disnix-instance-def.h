#ifndef __DISNIX_INSTANCE_DEF_H
#define __DISNIX_INSTANCE_DEF_H
#include <glib-object.h>

/** Enumeration of possible signals this service can emit */

typedef enum
{
    E_FINISH_SIGNAL,
    E_SUCCESS_SIGNAL,
    E_FAILURE_SIGNAL,
    E_LAST_SIGNAL
}
DisnixSignalNumber;

/** Captures the state of a D-Bus instance */
typedef struct
{
    /* Represents the parent class object state */
    GObject parent;
    
    /* Every D-Bus method returns a PID */
    gchar *pid;
}
DisnixObject;

/** Captures the state of the class of the D-Bus object */
typedef struct 
{
    /* Represents the state of the parent class */
    GObjectClass parent;
    /* Array of signals created for this class */
    guint signals[E_LAST_SIGNAL];
}
DisnixObjectClass;

#endif
