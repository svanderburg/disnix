#include <distributionexport.h>
#include <stdio.h>

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
	DistributionList *list;
	unsigned int i;
	
	/* Open the XML document */
	doc = create_distribution_export_doc(argv[1]);
	
	/* Check inter-dependencies */
	if(!checkInterDependencies(doc))
	{
	    fprintf(stderr, "Distribution export file has an inter-dependency error!\n");
	    fprintf(stderr, "Check if all inter-dependencies are present in the distribution!\n");
	    
	    xmlFreeDoc(doc);
	    xmlCleanupParser();
	    return 1;
	}
	
	/* Generate a distribution list */
	list = generate_distribution_list(doc);
	
	/* Distribute services in the distribution list */
	for(i = 0; i < list->size; i++)
	    printf("service: %s, target: %s\n", list->service[i], list->target[i]);
	
	/* Clean up */
	delete_distribution_list(list);    
	xmlFreeDoc(doc);
	xmlCleanupParser();
    
	return 0;
    }
}
