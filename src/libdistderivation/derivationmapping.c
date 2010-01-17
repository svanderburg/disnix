#include "derivationmapping.h"
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

GArray *create_derivation_array(char *distributed_derivation_file)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    xmlXPathObjectPtr result;
    GArray *derivation_array = NULL;
    unsigned int i;
    
    /* Parse the XML document */
    
    if((doc = xmlParseFile(distributed_derivation_file)) == NULL)
    {
	fprintf(stderr, "Error with parsing the distributed derivation XML file!\n");
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

    /* Query the mapping elements */
    result = executeXPathQuery(doc, "/distributedderivation/mapping");
    
    /* Iterate over all the mapping elements and add them to the array */
    
    if(result)
    {
	xmlNodeSetPtr nodeset = result->nodesetval;
	
	/* Create a derivation array */
        derivation_array = g_array_new(FALSE, FALSE, sizeof(DerivationItem*));
	
	/* Iterate over all the mapping elements */
	for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr mapping_children = nodeset->nodeTab[i]->children;
	    DerivationItem *item = (DerivationItem*)g_malloc(sizeof(DerivationItem));
	    gchar *derivation, *target;
	    
	    /* Iterate over all the mapping item children (derivation and target elements) */
	    
	    while(mapping_children != NULL)
	    {
		if(xmlStrcmp(mapping_children->name, (xmlChar*) "derivation") == 0)
		    derivation = g_strdup(mapping_children->children->content);
		else if(xmlStrcmp(mapping_children->name, (xmlChar*) "target") == 0)
		    target = g_strdup(mapping_children->children->content);
		
		mapping_children = mapping_children->next;
	    }
	    
	    /* Added the mapping to the array */
	    item->derivation = derivation;
	    item->target = target;
	    g_array_append_val(derivation_array, item);
        }
    }
    
    /* Cleanup */    
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);

    /* Return the derivation array */
    return derivation_array;
}

void delete_derivation_array(GArray *derivation_array)
{
    unsigned int i;
    
    for(i = 0; i < derivation_array->len; i++)
    {
	DerivationItem *item = g_array_index(derivation_array, DerivationItem*, i);
	g_free(item->derivation);
	g_free(item->target);
	g_free(item);
    }
    
    g_array_unref(derivation_array);
}
