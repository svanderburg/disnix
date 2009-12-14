#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <glib.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct
{
    gchar *profile;
    gchar *target;
}
DistributionItem;

static xmlXPathObjectPtr executeXPathQuery(xmlDocPtr doc, char *xpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    xmlXPathFreeContext(context);
    
    if(xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        xmlXPathFreeObject(result);
        return NULL;
    }
    else
	return result;
}

static GArray *generate_distribution_array(char *distribution_export_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *distribution_array = NULL;
    unsigned int i;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(distribution_export_file)) == NULL)
    {
	fprintf(stderr, "Error with parsing the distribution export XML file!\n");
	return NULL;
    }

    /* Retrieve root element */
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        fprintf(stderr, "The distribution export XML file is empty!\n");
	xmlFreeDoc(doc);
	return NULL;
    }
    
    /* Query the distribution elements */
    result = executeXPathQuery(doc, "/distributionexport/distribution/mapping");
    
    /* Iterate over all the distribution elements and add them to the array */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
	
	/* Create a distribution array */
        distribution_array = g_array_new(FALSE, FALSE, sizeof(DistributionItem*));
    
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    DistributionItem *item = (DistributionItem*)g_malloc(sizeof(DistributionItem));
	    gchar *profile, *target;
	    
	    /* Iterate over all the mapping item children (profile and target elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "profile") == 0)
		    profile = g_strdup(mapping_children->children->content);
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup(mapping_children->children->content);
		
		mapping_children = mapping_children->next;
	    }
	    
	    /* Added the mapping to the array */
	    item->profile = profile;
	    item->target = target;
	    g_array_append_val(distribution_array, item);
        }
    }
    
    /* Cleanup */    
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);

    /* Return the distribution array */
    return distribution_array;
}

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
	int exit_status = 0;
	unsigned int i;
	GArray *distribution_array;
	
	/* Generate a distribution array from the distribution export file */
	distribution_array = generate_distribution_array(argv[optind]);
	
	/* Iterate over the distribution array and distribute the profiles to the target machines */
	for(i = 0; i < distribution_array->len; i++)
	{
	    DistributionItem *item = g_array_index(distribution_array, DistributionItem*, i);
	    int status;
	    gchar *command;
	    
	    fprintf(stderr, "Distributing intra-dependency closure of profile: %s to target: %s\n", item->profile, item->target);
	    command = g_strconcat("disnix-copy-closure --target ", item->target, interface_arg, " ", item->profile, NULL);
	    status = system(command);
	    
	    /* Cleanups */
	    g_free(command);
	    g_free(item); /* Free the distribution item, since we don't need it anymore */
	    
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
	g_array_free(distribution_array, TRUE);
	g_free(interface_arg);
	xmlCleanupParser();
    
	/* Return the exit status, which is 0 if everything succeeds */
	return exit_status;
    }
}
