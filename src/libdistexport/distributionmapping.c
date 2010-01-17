#include "distributionmapping.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>

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

GArray *generate_distribution_array(char *distribution_export_file)
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
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
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

void delete_distribution_array(GArray *distribution_array)
{
    unsigned int i;
    
    for(i = 0; i < distribution_array->len; i++)
    {
	DistributionItem* item = g_array_index(distribution_array, DistributionItem*, i);
    
	g_free(item->profile);
	g_free(item->target);
	g_free(item);
    }
    
    g_array_unref(distribution_array);
}
