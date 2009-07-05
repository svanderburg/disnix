#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <glib.h>

static xmlXPathObjectPtr executeXPathQuery(xmlDocPtr doc, char *xpath)
{
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    
    if(xmlXPathNodeSetIsEmpty(result->nodesetval))
    {
        xmlXPathFreeObject(result);
        return NULL;
    }
    else
	return result;
}

static xmlXPathObjectPtr select_all_services_from_distribution(xmlDocPtr doc)
{
    return executeXPathQuery(doc, "/expr/list/attrs/attr[@name='service']/string/@value");
}

static xmlXPathObjectPtr select_services_from_distribution(xmlDocPtr doc, gchar *service)
{
    gchar *query = g_strconcat("/expr/list/attrs/attr[@name='service']/string[@value='", service, "']/@value", NULL);
    xmlXPathObjectPtr result = executeXPathQuery(doc, query);    
    g_free(query);
    return result;
}

static xmlXPathObjectPtr select_inter_dependencies(xmlDocPtr doc, gchar *service)
{
    gchar *query = g_strconcat("/expr/list/attrs/attr[@name='service']/string[@value='", service, "']/../../attr[@name='dependsOn']/list/string/@value", NULL);
    xmlXPathObjectPtr result = executeXPathQuery(doc, query);    
    g_free(query);
    return result;
}

static int checkInterDependencies(xmlDocPtr doc)
{
    int i, j, ret = TRUE;
    xmlXPathObjectPtr services_result = select_all_services_from_distribution(doc);
    
    if(services_result)
    {
	xmlNodeSetPtr nodeset = services_result->nodesetval;
	
	for(i = 0; i < nodeset->nodeNr; i++)
	{
	    xmlNodePtr service_node = nodeset->nodeTab[i]->children;
	    
	    while(service_node != NULL)
	    {
		xmlXPathObjectPtr interdeps_result = select_inter_dependencies(doc, service_node->content);	
		
		if(interdeps_result)
		{
		    xmlNodeSetPtr nodeset = interdeps_result->nodesetval;
		    
		    for(j = 0; j < nodeset->nodeNr; j++)
		    {
			xmlNodePtr interdep_node = nodeset->nodeTab[j]->children;
			
			while(interdep_node != NULL)
			{
			    xmlXPathObjectPtr query_result = select_services_from_distribution(doc, interdep_node->content);
			    
			    if(query_result == NULL)
			    {
				fprintf(stderr, "Inter-dependency: %s not found for service: %s\n", interdep_node->content, service_node->content);
				ret = FALSE;
			    }
			    
			    xmlXPathFreeObject(query_result);
			    			    
			    interdep_node = interdep_node->next;
			}
		    }
		}
		
		xmlXPathFreeObject(interdeps_result);
		
		service_node = service_node->next;
	    }
	}
    }
    
    xmlXPathFreeObject(services_result);
    
    return ret;
}

static xmlDocPtr create_distribution_export_doc(char *filename)
{
    xmlDocPtr doc;
    xmlNodePtr node_root;
    
    if((doc = xmlParseFile(filename)) == NULL)
    {
	fprintf(stderr, "Error with parsing the distribution export XML file!\n");
	return NULL;
    }
    
    node_root = xmlDocGetRootElement(doc);
    
    if(node_root == NULL)
    {
        fprintf(stderr, "The distribution export XML file is empty!\n");
	xmlFreeDoc(doc);
	return NULL;
    }
    
    return doc;
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
	fprintf(stderr, "No distribution export file specified!\n");
	return 1;
    }
    else
    {
	xmlDocPtr doc;
	
	doc = create_distribution_export_doc(argv[1]);
	
	if(!checkInterDependencies(doc))
	{
	    fprintf(stderr, "Distribution export file has an inter-dependency error!\n");
	    fprintf(stderr, "Check if all inter-dependencies are present in the distribution!\n");
	    return 1;
	}
    
	xmlFreeDoc(doc);
	xmlCleanupParser();
    
	return 0;
    }
}
