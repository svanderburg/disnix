#include "distributionlist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define CAPACITY_INCREMENT 10
#define TRUE 1
#define FALSE 0

static void increase_capacity(DistributionList *list)
{
    list->capacity += CAPACITY_INCREMENT;
    list->service = (char**)realloc(list->service, list->capacity * sizeof(char*));
    list->target = (char**)realloc(list->target, list->capacity * sizeof(char*));
    list->type = (char**)realloc(list->type, list->capacity * sizeof(char*));
    list->visited = (int*)realloc(list->visited, list->capacity * sizeof(int));
}

DistributionList *new_distribution_list()
{
    DistributionList *list = (DistributionList*)malloc(sizeof(DistributionList));
    list->capacity = 0;
    list->size = 0;
    list->service = NULL;
    list->target = NULL;
    list->type = NULL;
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
	free(list->type[i]);
    }
    
    free(list->service);
    free(list->target);
    free(list->type);
    free(list->visited);
    free(list);
}

void add_distribution_item(DistributionList *list, char *service, char *target, char *type)
{
    if(list->size == list->capacity)
	increase_capacity(list);

    list->service[list->size] = (char*)malloc((strlen(service) + 1) * sizeof(char));
    strcpy(list->service[list->size], service);
    list->target[list->size] = (char*)malloc((strlen(target) + 1) * sizeof(char));
    strcpy(list->target[list->size], target);
    list->type[list->size] = (char*)malloc((strlen(type) + 1) * sizeof(char));
    strcpy(list->type[list->size], type);
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
	    add_distribution_item(ret, list1->service[i], list1->target[i], list1->type[i]);
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
	    add_distribution_item(ret, list1->service[i], list1->target[i], list1->type[i]);
    }
    
    return ret;
}

static int compare(char *service1, char *target1, char *service2, char *target2)
{
    int status = strcmp(target1, target2);
    
    if(status == 0)
	return strcmp(service1, service2);
    else
	return status;
}

int distribution_item_index(DistributionList *list, char *service, char *target)
{
    int left = 0;
    int right = list->size - 1;
    
    while(left <= right)
    {
	int mid = (left + right) / 2;
        int status = compare(list->service[mid], list->target[mid], service, target);
	
	if(status == 0)
            return mid; /* Return index of the found distribution item */
	else if(status > 0)
	    right = mid - 1;
	else if(status < 0)
	    left = mid + 1;
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
	    add_distribution_item(ret, list->service[i], list->target[i], list->type[i]);
    }
    
    return ret;
}

void print_distribution_list(DistributionList *list)
{
    unsigned int i;
    
    for(i = 0; i < list->size; i++)
	printf("Service: %s, Target: %s, Type: %s\n", list->service[i], list->target[i], list->type[i]);
}

static int partition(DistributionList *list, int left, int right)
{
    int i = left, j = right;
    int mid = (left + right) / 2;
    char *pivot_service = list->service[mid];
    char *pivot_target = list->target[mid];
    
    while(i <= j)
    {
        while(compare(list->service[i], list->target[i], pivot_service, pivot_target) < 0)
	    i++;
	while(compare(list->service[j], list->target[j], pivot_service, pivot_target) > 0)
	    j--;
	
	if(i <= j)
	{
	    char *tmp_service = list->service[i];
	    char *tmp_target = list->target[i];
	    
	    list->service[i] = list->service[j];
	    list->target[i] = list->target[j];
	    
	    list->service[j] = tmp_service;
	    list->target[j] = tmp_target;
	    
	    i++;
	    j--;
	}
    }
    
    return i;
}

static void quick_sort(DistributionList *list, int left, int right)
{
    int index = partition(list, left, right);
    if(left < index - 1)
	quick_sort(list, left, index - 1);
    if(index < right)
	quick_sort(list, index, right);
}

void sort(DistributionList *list)
{
    quick_sort(list, 0, list->size - 1);
}
