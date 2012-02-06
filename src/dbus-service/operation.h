/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2011  Sander van der Burg
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

#ifndef __DISNIX_OPERATION_H
#define __DISNIX_OPERATION_H

/**
 * Enumeration of possible Disnix service operations
 */

typedef enum
{
    OP_NONE,
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
    OP_LOCK,
    OP_UNLOCK,
}
Operation;

#endif
