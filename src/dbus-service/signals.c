/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2010  Sander van der Burg
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

#include "signals.h"
#include "disnix-gtype-def.h"
#include <glib-object.h>
#include <glib.h>

gboolean disnix_emit_finish_signal(DisnixObject *obj, gint pid)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_FINISH_SIGNAL], 0, pid);
    
    return TRUE;
}

gboolean disnix_emit_success_signal(DisnixObject *obj, gint pid, gchar **derivation)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_SUCCESS_SIGNAL], 0, pid, derivation);

    return TRUE;
}

gboolean disnix_emit_failure_signal(DisnixObject *obj, gint pid)
{
    DisnixObjectClass *klass = DISNIX_OBJECT_GET_CLASS(obj);
    g_signal_emit(obj, klass->signals[E_FAILURE_SIGNAL], 0, pid);
    
    return TRUE;
}
