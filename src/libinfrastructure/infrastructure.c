/*
 * Disnix - A Nix-based distributed service deployment tool
 * Copyright (C) 2008-2016  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "infrastructure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <xmlutil.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#define BUFFER_SIZE 4096

static gint compare_target_property(const TargetProperty **l, const TargetProperty **r)
{
    const TargetProperty *left = *l;
    const TargetProperty *right = *r;
    
    return g_strcmp0(left->name, right->name);
}

static gchar *find_target_property(const GPtrArray *target, const gchar *name)
{
    TargetProperty key;
    const TargetProperty *key_ptr = &key;
    TargetProperty **ret;
    
    key.name = (gchar*)name;
    
    ret = bsearch(&key_ptr, target->pdata, target->len, sizeof(gpointer), (int (*)(const void *, const void *)) compare_target_property);
    
    if(ret == NULL)
        return NULL;
    else
        return (*ret)->value;
}

static gint compare_target(const GPtrArray **l, const GPtrArray **r)
{
    const GPtrArray *left = *l;
    const GPtrArray *right = *r;
    
    gchar *left_target_property = find_target_key(left, NULL);
    gchar *right_target_property = find_target_key(right, NULL);
    
    return g_strcmp0(left_target_property, right_target_property);
}

static xmlDocPtr create_infrastructure_doc(gchar *infrastructureXML)
{
    /* Declarations */
    xmlDocPtr doc, transform_doc;
    xmlNodePtr root_node;
    xsltStylesheetPtr style;
    
    /* Parse XML file from XML string */
    doc = xmlParseMemory(infrastructureXML, strlen(infrastructureXML));
    
    if(doc == NULL)
    {
	g_printerr("Error with parsing infrastructure XML file!\n");
	return NULL;
    }
    
    /* Check if the document has a root */
    root_node = xmlDocGetRootElement(doc);
    
    if(root_node == NULL)
    {
        g_printerr("The infrastructure XML file is empty!\n");
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
    
    /* Return transformed XML document */
    return transform_doc;
}

static gchar *create_infrastructure_xml(gchar *infrastructure_expr)
{
    int pipefd[2];
        
    /* 
     * Execute nix-instantiate command to retrieve XML representation of the 
     * infrastructure model
     */     
    
    if(pipe(pipefd) == 0)
    {
	int status = fork();
    
	if(status == -1)
	{
	    g_printerr("Error with forking nix-instantiate process!\n");
	    close(pipefd[0]);
	    close(pipefd[1]);
	    return NULL;
	}
	else if(status == 0)
	{
	    char *const args[] = {"nix-instantiate", "--eval-only", "--strict", "--xml", infrastructure_expr, NULL};
	    close(pipefd[0]); /* Close read-end of pipe */
	    dup2(pipefd[1], 1); /* Attach write-end to stdout */
	    execvp("nix-instantiate", args);
	    _exit(1);
	    return NULL;
	}
	else
	{
	    gchar *infrastructureXML = g_strdup("");
	    ssize_t line_size;
	    char line[BUFFER_SIZE];
	    
	    close(pipefd[1]); /* Close write-end of pipe */
		
	    while((line_size = read(pipefd[0], line, BUFFER_SIZE - 1)) > 0)
	    {
	        gchar *old_infrastructureXML = infrastructureXML;

	        line[line_size] = '\0';
		infrastructureXML = g_strconcat(old_infrastructureXML, line, NULL);
		g_free(old_infrastructureXML);
	    }
	    
	    close(pipefd[0]);
	    
	    wait(&status);

	    if(WEXITSTATUS(status) == 0)
		return infrastructureXML;
	    else
	    {
		g_printerr("Error with executing nix-instantiate!\n");
		g_free(infrastructureXML);
		return NULL;
	    }
	}    
    }
    else
    {
	g_printerr("Error with creating pipe!\n");
	return NULL;
    }
}

GPtrArray *create_target_array(char *infrastructure_expr)
{
    /* Declarations */
    gchar *infrastructureXML;
    xmlDocPtr doc;
    xmlXPathObjectPtr result;
    GPtrArray *target_array = NULL;
    
    /* Open the XML output of nix-instantiate */
    infrastructureXML = create_infrastructure_xml(infrastructure_expr);
    
    if(infrastructureXML == NULL)
    {
        g_printerr("Error opening infrastructure XML file!\n");
        return NULL;
    }
    
    /* Parse the infrastructure XML file */
    doc = create_infrastructure_doc(infrastructureXML);
    g_free(infrastructureXML);
    
    if(doc == NULL)
    {
        g_printerr("Error with parsing infrastructure XML file!\n");
        return NULL;
    }

    /* Query all the target properties from the XML document */
    result = executeXPathQuery(doc, "/infrastructure/target");

    /* Iterate over all targets and add them to the array */
    
    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
	target_array = g_ptr_array_new();
	
	/* Iterate over all target elements */
        for(i = 0; i < nodeset->nodeNr; i++)
        {
	    xmlNodePtr targets_children = nodeset->nodeTab[i]->children;
	    GPtrArray *target_properties = g_ptr_array_new();
	    
	    while(targets_children != NULL)
	    {
	        TargetProperty *targetProperty = (TargetProperty*)g_malloc(sizeof(TargetProperty));
	        targetProperty->name = g_strdup((gchar*)targets_children->name);
	        
	        if(targets_children->children == NULL)
	            targetProperty->value = NULL;
	        else
	            targetProperty->value = g_strdup((gchar*)targets_children->children->content);
	        
	        /* Add the property to the target properties array */
	        g_ptr_array_add(target_properties, targetProperty);
	        
	        targets_children = targets_children->next;
	    }
	    
	    /* Sort the target properties */
	    g_ptr_array_sort(target_properties, (GCompareFunc)compare_target_property);
	    
	    /* Add the property to the targets array */
	    g_ptr_array_add(target_array, target_properties);
	}
	
	/* Sort the targets array */
	g_ptr_array_sort(target_array, (GCompareFunc)compare_target);
	
	xmlXPathFreeObject(result);
    }
    else
        g_printerr("No targets found!\n");

    /* Cleanup */
    xmlFreeDoc(doc);
    xmlCleanupParser();

    /* Return the target array */
    return target_array;
}

void delete_target_array(GPtrArray *target_array)
{
    if(target_array != NULL)
    {
        unsigned int i;
    
        for(i = 0; i < target_array->len; i++)
        {
            GPtrArray *target_properties = g_ptr_array_index(target_array, i);
            unsigned int j;
            
            for(j = 0; j < target_properties->len; j++)
            {
                TargetProperty *target_property = g_ptr_array_index(target_properties, j);
                g_free(target_property->name);
                g_free(target_property->value);
                g_free(target_property);
            }
            
            g_ptr_array_free(target_properties, TRUE);
        }

        g_ptr_array_free(target_array, TRUE);
    }
}

gchar *find_target_key(const GPtrArray *target, const gchar *global_target_property)
{
    gchar *target_property_name = find_target_property(target, "targetProperty");
    
    if(target_property_name == NULL)
        return find_target_property(target, global_target_property);
    else
        return find_target_property(target, target_property_name);
}

gchar *find_client_interface(const GPtrArray *target)
{
    return find_target_property(target, "clientInterface");
}
