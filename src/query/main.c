#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-query [--interface interface] [--target-property targetProperty] [{-p | --profile} profile] infrastructure_expr\n");
    fprintf(stderr, "disnix-query {-h | --help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"target-property", required_argument, 0, 't'},
	{"profile", required_argument, 0, 'p'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    char *target_property = NULL;
    char *profile = "default";
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "p:h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 't':
		target_property = optarg;
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
    if(interface == NULL)
    {
	char *interface_env = getenv("DISNIX_CLIENT_INTERFACE");
	
	if(interface_env == NULL)
	    interface = "disnix-client";
	else
	    interface = interface_env;
    }

    if(target_property == NULL)
    {
	char *target_property_env = getenv("DISNIX_TARGET_PROPERTY");
	
	if(target_property_env == NULL)
	    target_property = "hostname";
	else
	    target_property = target_property_env;
    }

    if(optind >= argc)
    {
	fprintf(stderr, "A infrastructure Nix expression has to be specified!\n");
	return 1;
    }
    else
    {
	GArray *target_array = create_target_array(argv[optind], target_property);
	
	if(target_array != NULL)
	{
	    unsigned int i;
	    
	    for(i = 0; i < target_array->len; i++)
	    {
		gchar *target = g_array_index(target_array, gchar*, i);
		gchar *command = g_strconcat(interface, " --target ", target, " --profile ", profile, " --query-installed", NULL);
		int status;
		
		printf("Services on: %s\n", target);
		printf("--------------------------------------------------------------\n");
		
		status = system(command);						
		g_free(command);
		
		if(status == -1)
	    	    return -1;
		else if(WEXITSTATUS(status) != 0)
	    	    return WEXITSTATUS(status);
	    }
	    
	    delete_target_array(target_array);
	}

	return 0;
    }
}
