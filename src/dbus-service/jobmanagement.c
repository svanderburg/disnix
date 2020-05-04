/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2020  Sander van der Burg
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

#include "jobmanagement.h"
#include <stdlib.h>
#include <dirent.h>

/* Provides each job a unique job id */
int job_counter;

static int numbersort(const struct dirent **a, const struct dirent **b)
{
    int left = atoi((*a)->d_name);
    int right = atoi((*b)->d_name);
    
    if(left < right)
        return -1;
    else if(left > right)
        return 1;
    else
        return 0;
}

void determine_next_pid(char *logdir)
{
    struct dirent **namelist;
    int num_of_entries = scandir(logdir, &namelist, 0, numbersort);
    
    if(num_of_entries <= 0)
       job_counter = 0;
    else
    {
       char *filename = namelist[num_of_entries - 1]->d_name;
       job_counter = atoi(filename);
       job_counter++;
       free(namelist[num_of_entries - 1]);
       free(namelist);
    }
}

int assign_pid(void)
{
    int return_value = job_counter;
    job_counter++;
    return return_value;
}
