#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#define BUFFER_SIZE 4096

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-collect-garbage [--interface interface] [--target-property targetProperty] [-d|--delete-old] infrastructure_expr\n");
    fprintf(stderr, "disnix-activate {-h | --help}\n");
}

static xmlDocPtr create_infrastructure_doc(gchar *infrastructureXML)
{
    xmlDocPtr doc, transform_doc;
    xmlNodePtr root_node;
    xsltStylesheetPtr style;
    
    /* Parse XML file from XML string */
    doc = xmlParseMemory(infrastructureXML, strlen(infrastructureXML));
    
    if(doc == NULL)
    {
	fprintf(stderr, "Error with parsing infrastructure XML file!\n");
	return NULL;
    }
    
    /* Check if the document has a root */
    root_node = xmlDocGetRootElement(doc);
    
    if(root_node == NULL)
    {
        fprintf(stderr, "The infrastructure XML file is empty!\n");
	xmlFreeDoc(doc);
	return NULL;
    }

    /* Transform the document into a more concrete format */
    style = xsltParseStylesheetFile((const xmlChar *) DATADIR "/infrastructure.xsl");
    
    transform_doc = xsltApplyStylesheet(style, doc, NULL);
        
    /* Cleanup */
    xsltFreeStylesheet(style);    
    xmlFreeDoc(doc);
    xsltCleanupGlobals();
        
    return transform_doc;
}

static gchar *create_infrastructure_xml(char *infrastructure_expr)
{
    FILE *fp;
    int status;
    char line[BUFFER_SIZE];
    gchar *infrastructureXML = g_strdup("");
    gchar *command = g_strconcat("nix-instantiate --eval-only --xml ", infrastructure_expr, NULL);
    
    fp = popen(command, "r");
    g_free(command);
    
    while(fgets(line, sizeof(line), fp) != NULL)
    {
	gchar *old_infrastructureXML = infrastructureXML;
	infrastructureXML = g_strconcat(old_infrastructureXML, line, NULL);
	g_free(old_infrastructureXML);
    }
    
    status = pclose(fp);    
    
    if(status == -1 || WEXITSTATUS(status) != 0)
	return NULL;
    else
	return infrastructureXML;
}

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

static xmlXPathObjectPtr query_targets(xmlDocPtr doc, char *target_property)
{
    gchar *query = g_strconcat("/infrastructure/target/", target_property, NULL);
    xmlXPathObjectPtr result = executeXPathQuery(doc, query);
    g_free(query);
    return result;
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"interface", required_argument, 0, 'i'},
	{"target-property", required_argument, 0, 't'},
	{"delete-old", no_argument, 0, 'd'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    gchar *interface = NULL;
    char *delete_old_arg = "";
    char *targetProperty = "hostname";
    
    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "dh", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'i':
		interface = optarg;
		break;
	    case 't':
		targetProperty = optarg;
		break;
	    case 'd':
	        delete_old_arg = "-d";
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
	fprintf(stderr, "A infrastructure Nix expression has to be specified!\n");
	return 1;
    }
    else
    {
	char *infrastructure_expr = argv[optind];
	gchar *infrastructureXML;
	xmlDocPtr doc;
	xmlXPathObjectPtr result;
    
	/* Open the XML output of nix-instantiate */
    
	infrastructureXML = create_infrastructure_xml(infrastructure_expr);
    
	if(infrastructureXML == NULL)
	{
	    fprintf(stderr, "Error opening infrastructure XML file!\n");
	    return 1;
	}
    
	/* Parse the infrastructure XML file */
    
	doc = create_infrastructure_doc(infrastructureXML);
	g_free(infrastructureXML);
	
	if(doc == NULL)
	{
	    fprintf(stderr, "Error with parsing infrastructure XML file!\n");    
	    return 1;
	}
    
	/* Iterate over all targets */
        
	result = query_targets(doc, targetProperty);
    
	if(result)
	{
	    unsigned int i;
	    xmlNodeSetPtr nodeset = result->nodesetval;
	
	    for(i = 0; i < nodeset->nodeNr; i++)
	    {
		xmlChar *target_value = nodeset->nodeTab[i]->children->content;
		
		gchar *command = g_strconcat(interface, " --target ", target_value, " --collect-garbage ", delete_old_arg, NULL);
		int status;
		
		printf("Collecting garbage on: %s\n", target_value);
		status = system(command);						
		g_free(command);
		
		if(status == -1)
		{
		    xmlXPathFreeObject(result);
		    xmlFreeDoc(doc);
		    xmlCleanupParser();
		    return -1;
		}
		else
		{
		    xmlXPathFreeObject(result);
		    xmlFreeDoc(doc);
		    xmlCleanupParser();
		    return WEXITSTATUS(status);
		}
	    }
	
	    xmlXPathFreeObject(result);
	}
	else
	    fprintf(stderr, "No targets found!\n");
	    
	/* Cleanup */	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return 0;
    }
}
