#include <distributionexport.h>
#include <stdio.h>
#include <getopt.h>
#include <glib.h>

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-activate [{-i | --interface} interface] [{-o|--old-export} distribution_export_file] distribution_export_file\n");
    fprintf(stderr, "disnix-activate {-h | --help}\n");
}

static void deactivate(xmlDocPtr doc, DistributionList *list, char *service, char *target)
{
    int index = distribution_item_index(list, service, target);
    
    if(index == -1)
	fprintf(stderr, "Mapping of service: %s to target: %s does not exists in distribution export file!\n", service, target);
    else
    {
	if(!list->visited[index])
	{
	    xmlXPathObjectPtr result = select_inter_dependend_services_from(doc, service);
	    
	    if(result)
	    {
		xmlNodeSetPtr nodeset = result->nodesetval;
		unsigned int i, j;
		
		for(i = 0; i < nodeset->nodeNr; i++)
		{
		    xmlNodePtr node = nodeset->nodeTab[i]->children;
		    xmlChar *inter_dependency = node->content;
		    DistributionList *mappings = select_distribution_items(list, inter_dependency);
		    
		    for(j = 0; j < mappings->size; j++)
			deactivate(doc, list, mappings->service[j], mappings->target[j]);
		    
		    delete_distribution_list(mappings);
		}
	    }
	    
	    xmlXPathFreeObject(result);
	
	    printf("Deactivate: %s on: %s\n", service, target);
	    list->visited[index] = TRUE;
	}
    }
}

static void activate(xmlDocPtr doc, DistributionList *list, char *service, char *target)
{
    int index = distribution_item_index(list, service, target);
    
    if(index == -1)
	fprintf(stderr, "Mapping of service: %s to target: %s does not exists in distribution export file!\n", service, target);
    else
    {
	if(!list->visited[index])
	{
	    xmlXPathObjectPtr result = select_inter_dependencies(doc, service);
	    
	    if(result)
	    {
		xmlNodeSetPtr nodeset = result->nodesetval;
		unsigned int i, j;
		
		for(i = 0; i < nodeset->nodeNr; i++)
		{
		    xmlNodePtr node = nodeset->nodeTab[i]->children;
		    xmlChar *inter_dependency = node->content;
		    DistributionList *mappings = select_distribution_items(list, inter_dependency);
		    
		    for(j = 0; j < mappings->size; j++)
			activate(doc, list, mappings->service[j], mappings->target[j]);
		    
		    delete_distribution_list(mappings);
		}
	    }
	    
	    xmlXPathFreeObject(result);
	
	    printf("Activate: %s on: %s\n", service, target);
	    list->visited[index] = TRUE;
	}
    }
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"old-export", required_argument, 0, 'o'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    gchar *interface_arg = g_strdup("");
    char *old_export_file = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "i:o:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface_arg = g_strconcat(" --interface ", optarg, NULL);
		break;
	    case 'o':
	        old_export_file = optarg;
	        break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    if(optind >= argc)
    {
	fprintf(stderr, "A distribution export file has to be specified!\n");
	g_free(interface_arg);
	return 1;
    }
    else
    {
	xmlDocPtr doc_old = NULL, doc_new;
	DistributionList *list_old, *list_new;
	DistributionList *list_intersection, *list_activate, *list_deactivate;
	unsigned int i;
	char *new_export_file = argv[optind];
	
	/* Open the XML document */
	doc_new = create_distribution_export_doc(new_export_file);

	if(old_export_file != NULL)
	{
	    doc_old = create_distribution_export_doc(old_export_file);
    
	    /* Check inter-dependencies */
	    if(!checkInterDependencies(doc_old))
	    {
    		fprintf(stderr, "Distribution export file has an inter-dependency error!\n");
    		fprintf(stderr, "Check if all inter-dependencies are present in the distribution!\n");
	    
    		xmlFreeDoc(doc_old);
    		xmlCleanupParser();
    		return 1;
	    }
	    
	    /* Generate a distribution list */
	    list_old = generate_distribution_list(doc_old);
        }
	else
	    list_old = new_distribution_list();
	
	/* Check inter-dependencies */
	if(!checkInterDependencies(doc_new))
	{
    	    fprintf(stderr, "Distribution export file has an inter-dependency error!\n");
	    fprintf(stderr, "Check if all inter-dependencies are present in the distribution!\n");
	    
    	    xmlFreeDoc(doc_new);
    	    xmlCleanupParser();
    	    return 1;
	}
    
	/* Generate a distribution list */
	list_new = generate_distribution_list(doc_new);

	printf("old:\n");
	print_distribution_list(list_old);
	printf("\nnew:\n");
	print_distribution_list(list_new);
	printf("\nintersect:\n");
	list_intersection = intersection(list_old, list_new);
	print_distribution_list(list_intersection);
	printf("\nto deactivate:\n");
	list_deactivate = substract(list_old, list_intersection);
	print_distribution_list(list_deactivate);
	printf("\nto activate:\n");
	list_activate = substract(list_new, list_intersection);
	print_distribution_list(list_activate);

	/* Deactivate old services interdependency closures */
	printf("Deactivating obsolete services from old configuration:\n");
	
	for(i = 0; i < list_deactivate->size; i++)
	{
	    printf("\n");
	    deactivate(doc_old, list_deactivate, list_deactivate->service[i], list_deactivate->target[i]);
	}
    
	/* Activate new services interdependency closures */
	printf("Activating new services from new configuration:\n");
	
	for(i = 0; i < list_activate->size; i++)
	{
	    printf("\n");
	    activate(doc_new, list_activate, list_activate->service[i], list_activate->target[i]);
	}
	
	/* Clean up */
	g_free(interface_arg);
	delete_distribution_list(list_old);
	delete_distribution_list(list_new);
	delete_distribution_list(list_intersection);
	delete_distribution_list(list_deactivate);
	delete_distribution_list(list_activate);
	xmlFreeDoc(doc_old);
	xmlFreeDoc(doc_new);
	xmlCleanupParser();
	return 0;
    }
}
