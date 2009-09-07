#include <distributionexport.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <glib.h>

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-distribute [--interface] interface] distribution_export\n");
    fprintf(stderr, "disnix-distribute {-h | --help}\n");
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
	xmlDocPtr doc;
	DistributionList *list;
	unsigned int i;
	gchar *profile_export_file = g_strconcat(argv[optind], "/profiles.xml", NULL);
	int exit_status = 0;
	
	/* Open the XML document */
	doc = create_distribution_export_doc(profile_export_file);
		
	/* Generate a distribution list */
	list = generate_distribution_list(doc);
	
	/* Distribute services in the distribution list */
	for(i = 0; i < list->size; i++)
	{
	    int status;
	    gchar *command;
	    
	    fprintf(stderr, "Distributing intra-dependency closure of service: %s to target: %s\n", list->service[i], list->target[i]);
	    command = g_strconcat("disnix-copy-closure --target ", list->target[i], interface_arg, " ", list->service[i], NULL);
	    status = system(command);
	    
	    g_free(command);
	    
	    /* On error stop the distribute process */
	    if(status == -1)
	    {
		exit_status = -1;
		break;
	    }
	    else if(WEXITSTATUS(status) != 0)
	    {
		exit_status = WEXITSTATUS(status);
		break;
	    }
	}
	
	/* Clean up */
	g_free(profile_export_file);
	g_free(interface_arg);
	delete_distribution_list(list);    
	xmlFreeDoc(doc);
	xmlCleanupParser();
    
	/* Return the exit status, which is 0 if everything succeeds */
	return exit_status;
    }
}
