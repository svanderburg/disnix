#include "infrastructure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#define BUFFER_SIZE 4096

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

GArray *create_target_array(char *infrastructure_expr, char *target_property)
{
    gchar *infrastructureXML, *query;
    xmlDocPtr doc;
    xmlXPathObjectPtr result;
    GArray *target_array = NULL;
    
    /* Open the XML output of nix-instantiate */
    
    infrastructureXML = create_infrastructure_xml(infrastructure_expr);
    
    if(infrastructureXML == NULL)
    {
        fprintf(stderr, "Error opening infrastructure XML file!\n");
        return NULL;
    }
    
    /* Parse the infrastructure XML file */
    
    doc = create_infrastructure_doc(infrastructureXML);
    g_free(infrastructureXML);
	
    if(doc == NULL)
    {
        fprintf(stderr, "Error with parsing infrastructure XML file!\n");    
        return NULL;
    }

    query = g_strconcat("/infrastructure/target/", target_property, NULL);
    result = executeXPathQuery(doc, query);
    g_free(query);

    /* Iterate over all targets */
            
    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
	target_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
	
        for(i = 0; i < nodeset->nodeNr; i++)
        {
    	    xmlChar *target_value = nodeset->nodeTab[i]->children->content;
	    gchar *target = g_strdup(target_value);
	    g_array_append_val(target_array, target);
	}
	
	xmlXPathFreeObject(result);
    }
    else
        fprintf(stderr, "No targets found!\n");

    /* Cleanup */	
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return target_array;
}

void delete_target_array(GArray *target_array)
{
    unsigned int i;
    
    for(i = 0; i < target_array->len; i++)
    {
	gchar *target = g_array_index(target_array, gchar*, i);
	g_free(target);
    }

    g_array_free(target_array, TRUE);
}
