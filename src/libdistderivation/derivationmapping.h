#ifndef __DERIVATIONMAPPING_H
#define __DERIVATIONMAPPING_H
#include <glib.h>

typedef struct
{
    gchar *derivation;
    gchar *target;
}
DerivationItem;

GArray *create_derivation_array(char *distributed_derivation_file);

void delete_derivation_array(GArray *derivation_array);

#endif
