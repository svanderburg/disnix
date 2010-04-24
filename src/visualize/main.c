#include <stdio.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <activationmapping.h>
#include <glib.h>

static void print_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "disnix-visualize manifest\n");
    fprintf(stderr, "disnix-visualize {-h | --help}\n");
}

static GHashTable *generate_cluster_table(GArray *activation_array)
{
    GHashTable *cluster_table = g_hash_table_new(g_str_hash, g_str_equal);
    unsigned int i;

    for(i = 0; i < activation_array->len; i++)
    {
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	gchar *target = get_target_interface(mapping);
	GArray *services_array = g_hash_table_lookup(cluster_table, target);
	
	if(services_array == NULL)
	{	
	    services_array = g_array_new(FALSE, FALSE, sizeof(gchar*));  
	    g_hash_table_insert(cluster_table, target, services_array);
	}
	
	g_array_append_val(services_array, mapping->service);
    }
    
    return cluster_table;
}

static void destroy_cluster_table(GHashTable *cluster_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;    

    g_hash_table_iter_init(&iter, cluster_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	GArray *services_array = (GArray*)value;
	g_array_free(services_array, TRUE);
    }
    
    g_hash_table_destroy(cluster_table);
}

static GHashTable *generate_edges_table(GArray *activation_array)
{
    GHashTable *edges_table = g_hash_table_new(g_str_hash, g_str_equal);
    unsigned int i;

    for(i = 0; i < activation_array->len; i++)
    {
	ActivationMapping *mapping = g_array_index(activation_array, ActivationMapping*, i);
	gchar *target = get_target_interface(mapping);
	gchar *mapping_key = g_strconcat(mapping->service, ":", target, NULL);
	GArray *dependency_array = g_hash_table_lookup(edges_table, mapping_key);	
	
	if(dependency_array == NULL)
	{
	    unsigned int j;
	    GArray *depends_on = mapping->depends_on;
	    
	    dependency_array = g_array_new(FALSE, FALSE, sizeof(gchar*));
	    
	    for(j = 0; j < depends_on->len; j++)
	    {
		Dependency *dependency = g_array_index(depends_on, Dependency*, j);
		ActivationMapping lookup;
		ActivationMapping *actual_mapping;
		gchar *mapping_value, *target;
		int actual_mapping_index;
		
		/* Find the activation mapping in the activation array */
		lookup.service = dependency->service;
		lookup.target = dependency->target;
		actual_mapping_index = activation_mapping_index(activation_array, &lookup);
		
		/* Retrieve the actual mapping */
		actual_mapping = g_array_index(activation_array, ActivationMapping*, actual_mapping_index);
		
		/* Get the target interface */
		target = get_target_interface(actual_mapping);
		
		/* Generate mapping value from the service and target property */
		mapping_value = g_strconcat(actual_mapping->service, ":", target, NULL);
		    
	    	g_array_append_val(dependency_array, mapping_value);
	    }
	    
	    g_hash_table_insert(edges_table, mapping_key, dependency_array);	
	}
    }

    
    return edges_table;
}

static void destroy_edges_table(GHashTable *edges_table)
{
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;    

    g_hash_table_iter_init(&iter, edges_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	gchar *mapping_key = (gchar*)key;
	GArray *dependency_array = (GArray*)value;
	unsigned int i;
	
	for(i = 0; i < dependency_array->len; i++)
	{
	    gchar *dep = g_array_index(dependency_array, gchar*, i);
	    g_free(dep);
	}
	
	g_array_free(dependency_array, TRUE);
	g_free(mapping_key);
    }
        
    g_hash_table_destroy(edges_table);
}

static void generate_graph(char *manifest_file)
{
    GArray *activation_array = create_activation_array(manifest_file);
    
    GHashTable *cluster_table = generate_cluster_table(activation_array);
    GHashTable *edges_table = generate_edges_table(activation_array);
    
    GHashTableIter iter;
    gpointer *key;
    gpointer *value;    
    int count = 0;

    g_print("digraph G {\n");
        
    /* Generate clusters with nodes */
        
    g_hash_table_iter_init(&iter, cluster_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	unsigned int i;
	GArray *cluster_array = (GArray*)value;
	gchar *target = (gchar*)key;
	
	g_print("subgraph cluster_%d {\n", count);
	g_print("style=filled;\n");
	g_print("node [style=filled,fillcolor=white,color=black];\n");
	
	for(i = 0; i < cluster_array->len; i++)	
	{
	    gchar *service = g_array_index(cluster_array, gchar*, i);
	    g_print("\"%s:%s\" [ label = \"%s\" ];\n", service, target, service+44);	    
	}
	
	g_print("label = \"%s\"\n", target);
	g_print("}\n");
	
	count++;
    }
    
    /* Generate edges */
    
    g_hash_table_iter_init(&iter, edges_table);
    while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&value)) 
    {
	unsigned int i;
	GArray *dependency_array = (GArray*)value;
	
	for(i = 0; i < dependency_array->len; i++)
	{
	    gchar *dep = g_array_index(dependency_array, gchar*, i);
	    g_print("\"%s\" -> \"%s\"\n", (gchar*)key, dep);
	}
    }
    
    g_print("}\n");
    
    destroy_cluster_table(cluster_table);
    destroy_edges_table(edges_table);
    delete_activation_array(activation_array);
}

int main(int argc, char *argv[])
{
    /* Declarations */
    int c, option_index = 0;
    struct option long_options[] =
    {
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };

    /* Parse command-line options */
    while((c = getopt_long(argc, argv, "h", long_options, &option_index)) != -1)
    {
	switch(c)
	{
	    case 'h':
		print_usage();
		return 0;
	}
    }

    if(optind >= argc)
    {
	fprintf(stderr, "ERROR: No manifest specified!\n");
	return 1;
    }
    else
    {
	generate_graph(argv[optind]);
	return 0;
    }
}
