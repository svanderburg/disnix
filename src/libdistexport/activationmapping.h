#ifndef __ACTIVATIONMAPPING_H
#define __ACTIVATIONMAPPING_H
#include <glib.h>

typedef struct
{
    gchar *name;
    gchar *value;
}
TargetProperty;

typedef struct
{
    gchar *service;
    GArray *target;
}
Dependency;

typedef struct
{
    gchar *service;
    GArray *target;
    gchar *targetProperty;
    gchar *type;
    GArray *depends_on;
    gboolean activated;
}
ActivationMapping;

GArray *create_activation_list(char *distribution_export_file);

gint activation_mapping_index(GArray *activation_list, ActivationMapping *keys);

GArray *intersect_activation_list(GArray *left, GArray *right);

GArray *merge_activation_list(GArray *left, GArray *right);

GArray *substract_activation_list(GArray *left, GArray *right);

gchar *generate_activation_arguments(GArray *target);

gchar *get_target_interface(ActivationMapping *mapping);

GArray *find_interdependend_mappings(GArray *list, ActivationMapping *mapping);

void print_activation_list(GArray *activation_list);

#endif
