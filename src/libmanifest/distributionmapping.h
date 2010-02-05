#ifndef __DISTRIBUTIONMAPPING_H
#define __DISTRIBUTIONMAPPING_H
#include <glib.h>

/**
 * Contains a mapping of a Nix profile to a disnix service target
 */
typedef struct
{
    /** Nix store path to the profile */
    gchar *profile;
    /** Address of a disnix service */
    gchar *target;
}
DistributionItem;

/**
 * Creates a new array with distribution items from a manifest file.
 *
 * @param manifest_file Path to the manifest XML file
 * @return GArray with DistributionItems
 */
GArray *generate_distribution_array(char *manifest_file);

/**
 * Deletes an array with distribution items.
 *
 * @param distribution_array Array with distribution items
 */
void delete_distribution_array(GArray *distribution_array);

#endif
