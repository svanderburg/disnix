#include "gptrarray-util.h"

void delete_array(GPtrArray *array, DeleteElementFunction delete_element)
{
    if(array != NULL)
    {
        unsigned int i;

        for(i = 0; i < array->len; i++)
        {
            gpointer element = g_ptr_array_index(array, i);
            delete_element(element);
        }

        g_ptr_array_free(array, TRUE);
    }
}

int check_array(const GPtrArray *array, CheckElementFunction check_element)
{
    unsigned int i;
    int status = TRUE;

    for(i = 0; i < array->len; i++)
    {
        gpointer element = g_ptr_array_index(array, i);

        if(!check_element(element))
            status = FALSE;
    }

    return status;
}
