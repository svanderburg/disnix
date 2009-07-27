#include "distributionexport.h"
#include <stdio.h>

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

xmlXPathObjectPtr select_all_services_from_distribution(xmlDocPtr doc)
{
    return executeXPathQuery(doc, "/expr/list/attrs/attr[@name='service']/string/@value");
}

xmlXPathObjectPtr select_services_from_distribution(xmlDocPtr doc, gchar *service)
{
    gchar *query = g_strconcat("/expr/list/attrs/attr[@name='service']/string[@value='", service, "']/@value", NULL);
    xmlXPathObjectPtr result = executeXPathQuery(doc, query);    
    g_free(query);
    return result;
}

xmlXPathObjectPtr select_inter_dependencies(xmlDocPtr doc, gchar *service)
{
    gchar *query = g_strconcat("/expr/list/attrs/attr[@name='service']/string[@value='", service, "']/../../attr[@name='dependsOn']/list/string/@value", NULL);
    xmlXPathObjectPtr result = executeXPathQuery(doc, query);    
    g_free(query);
    return result;
}

xmlXPathObjectPtr select_inter_dependend_services_from(xmlDocPtr doc, gchar *service)
{
    gchar *query = g_strconcat("/expr/list/attrs/attr[@name='dependsOn']/list/string[@value='", service, "']/../../../attr[@name='service']/string/@value", NULL);
    xmlXPathObjectPtr result = executeXPathQuery(doc, query);    
    g_free(query);
    return result;
}

xmlXPathObjectPtr select_all_distribution_items(xmlDocPtr doc)
{
    return executeXPathQuery(doc, "/expr/list");
}

int checkInterDependencies(xmlDocPtr doc)
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

xmlDocPtr create_distribution_export_doc(char *filename)
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

DistributionList *generate_distribution_list(xmlDocPtr doc)
{
    int i;
    DistributionList *list = new_distribution_list();
    xmlXPathObjectPtr result = select_all_distribution_items(doc);

    if(result)
    {
        xmlNodeSetPtr nodeset = result->nodesetval;
	    
        for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr attrs_node = nodeset->nodeTab[i]->children;
		
	    while(attrs_node != NULL)
	    {
	        if(xmlStrcmp(attrs_node->name, (const xmlChar*) "attrs") == 0)
	        {
	    	    xmlNodePtr attr_node = attrs_node->children;
			
		    xmlChar *service = NULL;
		    xmlChar *target = NULL;

		    while(attr_node != NULL)
		    {
		        if(xmlStrcmp(attr_node->name, (const xmlChar*) "attr") == 0)
		        {
		    	    xmlAttrPtr attr_props = attr_node->properties;
				
			    while(attr_props != NULL)
			    {
			        xmlChar *attr_type = attr_props->children->content;
				    
			        if(xmlStrcmp(attr_type, (const xmlChar*) "service") == 0)
			        {
			   	    xmlNodePtr string_node = attr_node->children;
					
				    while(string_node != NULL)
				    {
				        if(xmlStrcmp(string_node->name, "string") == 0)
				    	    service = string_node->properties->children->content;
					
					string_node = string_node->next;
				    }
				}
				else if(xmlStrcmp(attr_type, (const xmlChar*) "target") == 0)
				{
				    xmlNodePtr string_node = attr_node->children;
					
				    while(string_node != NULL)
				    {
				        if(xmlStrcmp(string_node->name, "string") == 0)
				    	    target = string_node->properties->children->content;
					
					string_node = string_node->next;
				    }
				}
				    				    
				attr_props = attr_props->next;
			    }	
			}
			        		
			attr_node = attr_node->next;
		    }
			
		    add_distribution_item(list, service, target);
		}
			
		attrs_node = attrs_node->next;
	    }
	}
	
	xmlXPathFreeObject(result);
    }
    
    return list;
}
