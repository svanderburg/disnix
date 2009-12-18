#include <stdio.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "derivationmapping.h"

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
    gchar *interface_arg = NULL;
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface_arg = g_strconcat(" --interface ", optarg, NULL);
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
	
	if(interface_env == NULL)
	    interface_arg = g_strdup("");
	else
	    interface_arg = g_strconcat(" --interface ", interface_env, NULL);
    }

    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No distribution export specified!\n");
	g_free(interface_arg);
	return 1;
    }
    else
    {
	int exit_status = 0;
	GArray *derivation_array;
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
	    command = g_strconcat("disnix-copy-closure --to --target ", item->target, interface_arg, " ", item->derivation, NULL);
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
	for(i = 0; i < derivation_array->len; i++)
	{
	    DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
	    int status;
	    gchar *command;
	    
	    fprintf(stderr, "Realising derivation: %s on target: %s\n", item->derivation, item->target);
	    command = g_strconcat("disnix-client --realise --target ", item->target, interface_arg, " ", item->derivation, NULL);
	    status = system(command);
	    
	    /* Cleanups */
	    g_free(command);
	    
	    /* On error stop the process */
	    if(status == -1)
		return -1;
	    else if(WEXITSTATUS(status) != 0)
		return WEXITSTATUS(status);
	}
    }
}
