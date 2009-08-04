#ifndef __DISTRIBUTIONLIST_H
#define __DISTRIBUTIONLIST_H

typedef struct
{
    unsigned int size;
    unsigned int capacity;
    char **service;
    char **target;
    char **type;
    int *visited;
}
DistributionList;

DistributionList *new_distribution_list();

void delete_distribution_list(DistributionList *list);

void add_distribution_item(DistributionList *list, char *service, char *target, char *type);

DistributionList *intersection(DistributionList *list1, DistributionList *list2);

DistributionList *substract(DistributionList *list1, DistributionList *list2);

int distribution_item_index(DistributionList *list, char *service, char *target);

DistributionList *select_distribution_items(DistributionList *list, char *service);

void print_distribution_list(DistributionList *list);

#endif
