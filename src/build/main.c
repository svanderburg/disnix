#include <stdio.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "derivationmapping.h"
#define BUFFER_SIZE 4096

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-build [--interface] interface] distributed_derivation\n");
    fprintf(stderr, "disnix-build {-h | --help}\n");
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    char *interface = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
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

    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No distribution export specified!\n");
	return 1;
    }
    else
    {
	int exit_status = 0;
	GArray *derivation_array;
	GArray *result_array;
	unsigned int i;
	
	/* Generate a distribution array from the distribution export file */
	derivation_array = create_derivation_array(argv[optind]);
	
	/* Iterate over the derivation array and distribute the store derivation closures to the target machines */
	for(i = 0; i < derivation_array->len; i++)
	{
	    DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
	    int status;
	    gchar *command;
	    
	    fprintf(stderr, "Distributing intra-dependency closure of derivation: %s to target: %s\n", item->derivation, item->target);
	    command = g_strconcat("disnix-copy-closure --to --target ", item->target, " --interface '", interface, "' ", item->derivation, NULL);
	    status = system(command);
	    
	    /* Cleanups */
	    g_free(command);
	    
	    /* On error stop the distribute process */
	    if(status == -1)
		return -1;
	    else if(WEXITSTATUS(status) != 0)
		return WEXITSTATUS(status);
	}

	/* Iterate over the derivation array and realise the store derivations on the target machines */
	
	result_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
	for(i = 0; i < derivation_array->len; i++)
	{
	    DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
	    int status;
	    gchar *command;
	    FILE *fp;
	    
	    fprintf(stderr, "Realising derivation: %s on target: %s\n", item->derivation, item->target);
	    command = g_strconcat(interface, " --realise --target ", item->target, " ", item->derivation, NULL);
	    fp = popen(command, "r");
	    
	    if(fp == NULL)
		return -1;
	    else
	    {
		char line[BUFFER_SIZE];
		
		while(fgets(line, sizeof(line), fp) != NULL)
		{
		    gchar *result;
		    
		    puts(line);
		    result = g_strdup(line);
		    g_array_append_val(result_array, result);
		}
	
		status = pclose(fp);

		/* On error stop the process */
		if(status == -1)
		    return -1;
		else if(WEXITSTATUS(status) != 0)
		    return WEXITSTATUS(status);
	    }
	    
	    /* Cleanups */
	    g_free(command);	    
	}
	
	/* Retrieve back the realised closures and import them into the Nix store of the host */
    
	for(i = 0; i < derivation_array->len; i++)
	{
	    gchar *command, *result;
	    DerivationItem *item;
	    int status;
	
	    result = g_array_index(result_array, gchar*, i);
	    item = g_array_index(derivation_array, DerivationItem*, i);
	
	    fprintf(stderr, "Copying result: %s from: %s\n", result, item->target);
	
	    command = g_strconcat("disnix-copy-closure --from --target ", item->target, " --interface '", interface, "' ", result, NULL);	    
	    status = system(command);

	    /* On error stop the process */
	    if(status == -1)
		return -1;
	    else if(WEXITSTATUS(status) != 0)
		return WEXITSTATUS(status);			    
	}
	
	/* Cleanup */
	
	delete_derivation_array(derivation_array);
    }
}
