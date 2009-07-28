#include "distributionlist.h"
#include <stdlib.h>
#include <string.h>
#define CAPACITY_INCREMENT 10
#define TRUE 1
#define FALSE 0

static void increase_capacity(DistributionList *list)
{
    list->capacity += CAPACITY_INCREMENT;
    list->service = (char**)realloc(list->service, list->capacity * sizeof(char*));
    list->target = (char**)realloc(list->target, list->capacity * sizeof(char*));
    list->visited = (int*)realloc(list->visited, list->capacity * sizeof(int));
}

DistributionList *new_distribution_list()
{
    DistributionList *list = (DistributionList*)malloc(sizeof(DistributionList));
    list->capacity = 0;
    list->size = 0;
    list->service = NULL;
    list->target = NULL;
    list->visited = NULL;
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
	free(list->visited[i]);
    }
    
    free(list->service);
    free(list->target);
    free(list->visited);
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
    list->visited[list->size] = FALSE;
    
    list->size++;
}

DistributionList *intersection(DistributionList *list1, DistributionList *list2)
{
    unsigned int i;
    DistributionList *ret = new_distribution_list();
    
    for(i = 0; i < list1->size; i++)
    {
	if(distribution_item_index(list2, list1->service[i], list1->target[i]) != -1)
	    add_distribution_item(ret, list1->service[i], list1->target[i]);
    }
    
    return ret;
}

DistributionList *substract(DistributionList *list1, DistributionList *list2)
{
    unsigned int i;
    DistributionList *ret = new_distribution_list();
    
    for(i = 0; i < list1->size; i++)
    {
	if(distribution_item_index(list2, list1->service[i], list1->target[i]) == -1)
	    add_distribution_item(ret, list1->service[i], list1->target[i]);
    }
    
    return ret;
}

int distribution_item_index(DistributionList *list, char *service, char *target)
{
    int i;
    
    for(i = 0; i < list->size; i++)
    {
	if(strcmp(service, list->service[i]) == 0 &&
	   strcmp(target, list->target[i]) == 0)
	    return i;
    }
    
    return -1; /* Distribution item not found */
}

DistributionList *select_distribution_items(DistributionList *list, char *service)
{
    unsigned int i;
    DistributionList *ret = new_distribution_list();
    
    for(i = 0; i < list->size; i++)
    {
	if(strcmp(service, list->service[i]) == 0)
	    add_distribution_item(ret, list->service[i], list->target[i]);
    }
    
    return ret;
}
