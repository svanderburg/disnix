/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __DISNIX_INSTANCE_H
#define __DISNIX_INSTANCE_H
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
    /** Represents the parent class object state */
    GObject parent;
    
    /** Every D-Bus method returns a PID */
    gint pid;
}
DisnixObject;

/** Captures the state of the class of the D-Bus object */
typedef struct 
{
    /** Represents the state of the parent class */
    GObjectClass parent;
    /** Array of signals created for this class */
    guint signals[E_LAST_SIGNAL];
}
DisnixObjectClass;

#endif
