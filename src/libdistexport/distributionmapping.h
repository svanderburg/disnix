#ifndef __DISTRIBUTIONMAPPING_H
#define __DISTRIBUTIONMAPPING_H
#include <glib.h>

typedef struct
{
    gchar *profile;
    gchar *target;
}
DistributionItem;

GArray *generate_distribution_array(char *distribution_export_file);

void delete_distribution_array(GArray *distribution_array);

#endif
