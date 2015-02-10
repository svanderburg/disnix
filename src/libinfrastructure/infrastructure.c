/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008-2013  Sander van der Burg
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

GPtrArray *create_target_array(char *infrastructure_expr, const char *target_property)
{
    /* Declarations */
    gchar *infrastructureXML, *query;
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
    query = g_strconcat("/infrastructure/target/", target_property, NULL);
    result = executeXPathQuery(doc, query);
    g_free(query);

    /* Iterate over all targets and add them to the array */
            
    if(result)
    {
        unsigned int i;
        xmlNodeSetPtr nodeset = result->nodesetval;
	target_array = g_ptr_array_new();
	
        for(i = 0; i < nodeset->nodeNr; i++)
        {
    	    xmlChar *target_value = nodeset->nodeTab[i]->children->content;
	    gchar *target = g_strdup((gchar*)target_value);
	    g_ptr_array_insert(target_array, -1, target);
	}
	
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
            gchar *target = g_ptr_array_index(target_array, i);
            g_free(target);
        }

        g_ptr_array_free(target_array, TRUE);
    }
}
