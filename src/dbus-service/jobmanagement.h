/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
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

#ifndef __DISNIX_JOBMANAGEMENT_H
#define __DISNIX_JOBMANAGEMENT_H

/**
 * Determines what the next job id would be by inspecting the log files stored
 * in the log directory.
 *
 * @param logdir Path to the log directory
 */
void determine_next_pid(char *logdir);

/**
 * Assigns a pid to a job.
 *
 * @return A numeric job id
 */
int assign_pid(void);

#endif
