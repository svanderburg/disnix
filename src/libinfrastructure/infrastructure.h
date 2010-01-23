#ifndef __INFRASTRUCTURE_H
#define __INFRASTRUCTURE_H
#include <glib.h>

GArray *create_target_array(char *infrastructure_expr, char *target_property);

void delete_target_array(GArray *target_array);

#endif
