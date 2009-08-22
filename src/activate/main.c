#include <distributionexport.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <glib.h>

typedef enum
{
    ACTIVATE,
    DEACTIVATE
}
TraversalType;

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-activate [{-i | --interface} interface] [{-o|--old-export} distribution_export_file] distribution_export_file\n");
    fprintf(stderr, "disnix-activate {-h | --help}\n");
}

static int traverse_inter_dependency_graph(char *interface, xmlDocPtr doc, DistributionList *list, char *service, char *target, char *type, TraversalType traversal_type)
{
    /* Search for the given distribution item in the list */
    int index = distribution_item_index(list, service, target);
    
    if(index == -1)
    {
	/* If the distribution item does not exists, abort and give an error message */
	fprintf(stderr, "Mapping of service: %s to target: %s does not exists in distribution export file!\n", service, target);
	return -1;
    }
    else
    {	
	if(!list->visited[index])
	{
	    int status;
	    gchar *command;
	    char *operation;
	    xmlXPathObjectPtr result = NULL;
	    	    
	    switch(traversal_type)
	    {
		case ACTIVATE:
		    result = select_inter_dependencies(doc, service);
		    break;
	        case DEACTIVATE:
		    result = select_inter_dependend_services_from(doc, service);
		    break;
	    }
	    
	    if(result)
	    {
		xmlNodeSetPtr nodeset = result->nodesetval;
		unsigned int i, j;
		int status;
		
		for(i = 0; i < nodeset->nodeNr; i++)
		{
		    xmlNodePtr node = nodeset->nodeTab[i]->children;
		    xmlChar *inter_dependency = node->content;
		    DistributionList *mappings = select_distribution_items(list, inter_dependency);
		    
		    for(j = 0; j < mappings->size; j++)
			status = traverse_inter_dependency_graph(interface, doc, list, mappings->service[j], mappings->target[j], mappings->type[j], traversal_type);
		    
		    delete_distribution_list(mappings);
		    
		    if(status != 0)
		    {
			xmlXPathFreeObject(result);
			return status;
		    }
		}
	    }
	    
	    xmlXPathFreeObject(result);
	
	    switch(traversal_type)
	    {
		case ACTIVATE:
		    operation = "activate";
		    break;
		case DEACTIVATE:
		    operation = "deactivate";
		    break;
	    }
	    
	    printf("%s: %s on: %s of type: %s\n", operation, service, target, type);
	    
	    command = g_strconcat(interface, " --", operation, " ", service, " --type ", type, " --target ", target, NULL);
	    status = system(command);
	    g_free(command);
	    
	    if(status == -1)
		return -1;
	    else if(WEXITSTATUS(status) != 0)
		return WEXITSTATUS(status);
			    
	    list->visited[index] = TRUE;
	}
	
	return 0;
    }
}

static int deactivate(char *interface, xmlDocPtr doc, DistributionList *list, char *service, char *target, char *type)
{
    return traverse_inter_dependency_graph(interface, doc, list, service, target, type, DEACTIVATE);
}

static int activate(char *interface, xmlDocPtr doc, DistributionList *list, char *service, char *target, char *type)
{
    return traverse_inter_dependency_graph(interface, doc, list, service, target, type, ACTIVATE);
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
    gchar *interface = "disnix-client";
    char *old_export = NULL;
    
    /* Get current username */
    char *username = (getpwuid(1000))->pw_name;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "i:o:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 'o':
	        old_export = optarg;
	        break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    if(optind >= argc)
    {
	fprintf(stderr, "A distribution export file has to be specified!\n");
	return 1;
    }
    else
    {
	xmlDocPtr doc_old = NULL, doc_new, doc_profiles;
	DistributionList *list_old, *list_new;
	DistributionList *list_intersection, *list_activate, *list_deactivate;
	DistributionList *list_profiles;
	unsigned int i;
	char *new_export = argv[optind];
	gchar *new_export_file = g_strconcat(new_export, "/export.xml", NULL);
	gchar *old_export_file = NULL;
	gchar *profiles_file = g_strconcat(new_export, "/profiles.xml", NULL);
	
	/* Open the XML document */
	doc_new = create_distribution_export_doc(new_export_file);
	g_free(new_export_file);
		
	if(old_export == NULL)
        {
	    /* If no old export file is given, try to to open the export file in the Nix profile */
	    
	    old_export_file = g_strconcat("/nix/var/nix/profiles/per-user/", username, "/disnix-coordinator/export.xml", NULL);
	    FILE *file = fopen(old_export_file, "r");
	    
	    if(file == NULL)
	    {
		g_free(old_export_file);
		old_export_file = NULL;
	    }
	    else
		fclose(file);
	}
	else
	    old_export_file = g_strconcat(old_export, "/export.xml", NULL);
	
	if(old_export_file != NULL)
	{	    
	    doc_old = create_distribution_export_doc(old_export_file);
	    g_free(old_export_file);
	    
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

	/* Open profiles document */
	doc_profiles = create_distribution_export_doc(profiles_file);
	g_free(profiles_file);
	list_profiles = generate_distribution_list(doc_profiles);

	/* Deactivate old services interdependency closures */
	printf("Deactivating obsolete services from old configuration:\n");
	
	for(i = 0; i < list_deactivate->size; i++)
	{
	    printf("\n");
	    
	    if(!deactivate(interface, doc_old, list_deactivate, list_deactivate->service[i], list_deactivate->target[i], list_deactivate->type[i]))
		return 1;
	}
	
	/* Activate new services interdependency closures */
	printf("Activating new services from new configuration:\n");
	
	for(i = 0; i < list_activate->size; i++)
	{
	    printf("\n");
	    
	    if(!activate(interface, doc_new, list_activate, list_activate->service[i], list_activate->target[i], list_activate->type[i]))
		return 1;
	}
	
	/* Set the new profiles on the target machines */
	
	printf("Setting the new profiles on the target machines:\n");
	
	for(i = 0; i < list_profiles->size; i++)
	{
	    gchar *command;
	    int status;
	    printf("Setting profile: %s on target: %s\n", list_profiles->service[i], list_profiles->target[i]);
	    
	    command = g_strconcat(interface, " --target ", list_profiles->target[i], " --set ", list_profiles->service[i], NULL);
	    status = system(command);	    	    	    
	    g_free(command);
	    
	    if(status == -1)
		return -1;
	    else if(WEXITSTATUS(status) != 0)
		return WEXITSTATUS(status);
	}
	
	/* Store the activated distribution export in the profile of the current user */
	
	{	    
	    gchar *command;
	    int status;
	    
	    printf("Setting the coordinator profile:\n");
	    command = g_strconcat("nix-env -p /nix/var/nix/profiles/per-user/", username, "/disnix-coordinator --set $(readlink -f ", new_export, ")", NULL);
	    status = system(command);
	    g_free(command);
	    
	    if(status == -1)
		return -1;
	    else if(WEXITSTATUS(status) != 0)
		return WEXITSTATUS(status);
	}
	
	/* Clean up */
	delete_distribution_list(list_old);
	delete_distribution_list(list_new);
	delete_distribution_list(list_profiles);
	delete_distribution_list(list_intersection);
	delete_distribution_list(list_deactivate);
	delete_distribution_list(list_activate);
	xmlFreeDoc(doc_old);
	xmlFreeDoc(doc_new);
	xmlFreeDoc(doc_profiles);
	xmlCleanupParser();
	return 0;
    }
}
