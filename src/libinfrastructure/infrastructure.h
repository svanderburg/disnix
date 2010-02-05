#ifndef __INFRASTRUCTURE_H
#define __INFRASTRUCTURE_H
#include <glib.h>

/**
 * Creates an array with target properties from an infrastructure Nix expression
 *
 * @param infrastructure_expr Path to the infrastructure Nix expression
 * @param target_property Idenifier of the property that specifies how to
 *                        connect to the remote disnix service
 * @return GArray with target properties
 */
GArray *create_target_array(char *infrastructure_expr, char *target_property);

/**
 * Deletes an array with target properties
 *
 * @param target_array Array to delete
 */
void delete_target_array(GArray *target_array);

#endif
