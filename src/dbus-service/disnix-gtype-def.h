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

/**
 * Forward declaration of the function that will return the GType of
 * the Value implementation. Not used in this program
 */
 
GType disnix_object_get_type (void);

/* 
 * Macro for the above. It is common to define macros using the
 * naming convention (seen below) for all GType implementations,
 * and that's why we're going to do that here as well.
 */
#define DISNIX_TYPE_OBJECT              (disnix_object_get_type ())
#define DISNIX_OBJECT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), DISNIX_TYPE_OBJECT, DisnixObject))
#define DISNIX_OBJECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), DISNIX_TYPE_OBJECT, DisnixObjectClass))
#define DISNIX_IS_OBJECT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), DISNIX_TYPE_OBJECT))
#define DISNIX_IS_OBJECT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), DISNIX_TYPE_OBJECT))
#define DISNIX_OBJECT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), DISNIX_TYPE_OBJECT, DisnixObjectClass))
