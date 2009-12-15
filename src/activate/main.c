#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include "activationmapping.h"

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-activate [--interface interface] [{-p|--profile} profile] [{-o|--old-export} distribution_export_file] distribution_export_file\n");
    fprintf(stderr, "disnix-activate {-h | --help}\n");
}

static void activate(GArray *union_list, ActivationMapping *mapping)
{
    gint actual_mapping_index = activation_mapping_index(union_list, mapping);
    ActivationMapping *actual_mapping = g_array_index(union_list, ActivationMapping*, actual_mapping_index);
    
    /* First activate all inter-dependencies */
    if(actual_mapping->depends_on != NULL)
    {
	unsigned int i;
	
	for(i = 0; i < actual_mapping->depends_on->len; i++)
	{
	    Dependency *dependency = g_array_index(actual_mapping->depends_on, Dependency*, i);
	    ActivationMapping lookup;
	    
	    lookup.service = dependency->service;
	    lookup.target = dependency->target;	    	    	    
	    
	    activate(union_list, &lookup);
	}
    }
    
    /* Finally activate the service itself */
    if(!actual_mapping->activated)
    {
	gchar *arguments = generate_activation_arguments(actual_mapping->target);
	gchar *target_interface = get_target_interface(actual_mapping);
	
	printf("Now activating service: %s of type: %s through: %s\n", actual_mapping->service, actual_mapping->type, target_interface);
	printf("Using arguments: %s\n", arguments);
	actual_mapping->activated = TRUE;
	
	g_free(arguments);
    }
}

static void deactivate(GArray *union_list, ActivationMapping *mapping)
{
    gint actual_mapping_index = activation_mapping_index(union_list, mapping);
    ActivationMapping *actual_mapping = g_array_index(union_list, ActivationMapping*, actual_mapping_index);
    GArray *interdependend_services = find_interdependend_mappings(union_list, actual_mapping);
    
    /* First deactivate all service which have an inter-dependency on this service */
        
    unsigned int i;
	
    for(i = 0; i < interdependend_services->len; i++)
    {
        ActivationMapping *dependency_mapping = g_array_index(interdependend_services, Dependency*, i);
	printf("found interdep: %s\n", dependency_mapping->service);
        deactivate(union_list, dependency_mapping);
    }
    
    /* Finally deactivate the service itself */
    if(actual_mapping->activated)
    {
	gchar *arguments = generate_activation_arguments(actual_mapping->target);
	gchar *target_interface = get_target_interface(actual_mapping);
	
	printf("Now deactivating service: %s of type: %s through: %s\n", actual_mapping->service, actual_mapping->type, target_interface);
	printf("Using arguments: %s\n", arguments);
	actual_mapping->activated = FALSE;
	
	g_free(arguments);
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
	{"profile", required_argument, 0, 'p'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    gchar *interface_arg = NULL;
    char *old_export = NULL;
    char *profile = "default";
    
    /* Get current username */
    char *username = (getpwuid(geteuid()))->pw_name;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "o:p:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface_arg = g_strconcat("--interface ", optarg, NULL);
		break;
	    case 'o':
	        old_export = optarg;
	        break;
	    case 'p':
	        profile = optarg;
		break;
	    case 'h':
		print_usage();
		return 0;
	}
    }

    /* Validate options */
    if(interface_arg == NULL)
    {
	char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");
	
	if(interface_env != NULL)
	    interface_arg = g_strconcat("--interface ", interface_env, NULL);
    }

    if(optind >= argc)
    {
	fprintf(stderr, "A distribution export file has to be specified!\n");
	g_free(interface_arg);
	return 1;
    }
    else
    {
	unsigned int i;
	gchar *old_export_file;
	GArray *list_new = create_activation_list(argv[optind]);	
	GArray *list_old;
	
	printf("new:\n");
	print_activation_list(list_new);
	
	if(old_export == NULL)
        {
	    /* If no old export file is given, try to to open the export file in the Nix profile */
	    
	    old_export_file = g_strconcat("/nix/var/nix/profiles/per-user/", username, "/disnix-coordinator/", profile, NULL);
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
	    old_export_file = g_strdup(old_export);

	if(old_export_file != NULL)
	{	    	    
	    printf("Using previous distribution export: %s\n", old_export_file);
	    list_old = create_activation_list(old_export_file);
	    g_free(old_export_file);
        }
	else
	    list_old = NULL;

	GArray *unio;
	GArray *deactivate_list;
	GArray *activate_list;
	
	if(list_old != NULL)
	{
	    printf("old:\n");
	    print_activation_list(list_old);

	    printf("intersect:\n");
    	    GArray *intsect = intersect_activation_list(list_new, list_old);
	    print_activation_list(intsect);
		    	    
	    printf("to deactivate:\n");
	    deactivate_list = substract_activation_list(list_old, intsect);
	    print_activation_list(deactivate_list);
	    
	    printf("to activate:\n");
	    activate_list = substract_activation_list(list_new, intsect);
	    print_activation_list(activate_list);

	    printf("union:\n");
	    unio = union_activation_list(list_old, list_new, intsect);
	    print_activation_list(unio);
	}	
	else	
	{
	    unio = list_new;
	    deactivate_list = NULL;
	    activate_list = list_new;
	}
	
	printf("Deactivate:\n");
	
	if(deactivate_list != NULL)
	{
	    for(i = 0; i < deactivate_list->len; i++)
	    {
		ActivationMapping *mapping = g_array_index(deactivate_list, ActivationMapping*, i);
		deactivate(unio, mapping);
	    }
	}
	
	printf("Activate:\n");
		
	for(i = 0; i < activate_list->len; i++)
	{
	    ActivationMapping *mapping = g_array_index(activate_list, ActivationMapping*, i);
	    activate(unio, mapping);
	}
	
	g_free(interface_arg);
	
	return 0;
    }
}
