#ifndef __DERIVATIONMAPPING_H
#define __DERIVATIONMAPPING_H
#include <glib.h>

/**
 * Contains a mapping of a Nix store derivation to a Disnix Service target
 */
 
typedef struct
{
    /** Nix store derivation path */
    gchar *derivation;
    /** Address of a disnix service */
    gchar *target;
}
DerivationItem;

/**
 * Creates a new array with derivation items from a distributed derivation
 * XML file.
 *
 * @param distributed_derivation_file Path to the distributed derivation XML file
 * @return GArray with DerivationItems
 */ 
GArray *create_derivation_array(char *distributed_derivation_file);

/**
 * Deletes an array with derivation items
 *
 * @param derivation_array Derivation array to delete
 */
void delete_derivation_array(GArray *derivation_array);

#endif
