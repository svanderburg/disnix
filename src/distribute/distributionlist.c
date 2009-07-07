#include "distributionlist.h"
#include <stdlib.h>
#include <string.h>
#define CAPACITY_INCREMENT 10

static void increase_capacity(DistributionList *list)
{
    list->capacity += CAPACITY_INCREMENT;
    list->service = (char**)realloc(list->service, list->capacity * sizeof(char*));
    list->target = (char**)realloc(list->target, list->capacity * sizeof(char*));
}

DistributionList *new_distribution_list()
{
    DistributionList *list = (DistributionList*)malloc(sizeof(DistributionList));
    list->capacity = 0;
    list->size = 0;
    list->service = NULL;
    list->target = NULL;
    increase_capacity(list);
        
    return list;
}

void delete_distribution_list(DistributionList *list)
{
    unsigned int i;
    
    for(i = 0; i < list->size; i++)
    {
	free(list->service[i]);
	free(list->target[i]);
    }
    
    free(list->service);
    free(list->target);
    free(list);
}

void add_distribution_item(DistributionList *list, char *service, char *target)
{
    if(list->size == list->capacity)
	increase_capacity(list);

    list->service[list->size] = (char*)malloc((strlen(service) + 1) * sizeof(char));
    strcpy(list->service[list->size], service);
    list->target[list->size] = (char*)malloc((strlen(target) + 1) * sizeof(char));
    strcpy(list->target[list->size], target);

    list->size++;
}
