/*
 * Copyright (c) 2019-2020 Sander van der Burg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nixxml-print-nix.h"
#include <string.h>
#include "nixxml-types.h"

#define TRUE 1
#define FALSE 0

void NixXML_print_string_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    const char *string_value = (const char*)value;
    unsigned int i = 0;

    fputc('\"', file);

    while(string_value[i] != '\0')
    {
        char c = string_value[i];

        switch(c)
        {
            case '\"':
            case '\\':
                fputc('\\', file);
                fputc(c, file);
                break;
            case '\n':
                fputc('\\', file);
                fputc('n', file);
                break;
            default:
                fputc(c, file);
        }

        i++;
    }

    fputc('\"', file);
}

void NixXML_print_value_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    fprintf(file, "%s", (const char*)value);
}

void NixXML_print_path_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    const char *string_value = (const char*)value;
    unsigned int i = 0;
    NixXML_bool has_spaces = FALSE;

    /* Check if the path contains whitespaces */

    while(string_value[i] != '\0')
    {
        if(string_value[i] == ' ')
        {
            has_spaces = TRUE;
            break;
        }

        i++;
    }


    /* Print the path */

    if(has_spaces)
    {
        fprintf(file, "/. +");
        NixXML_print_string_nix(file, value, indent_level, userdata);
    }
    else
    {
        if((strlen(string_value) > 0 && string_value[0] == '/') /* Check whether we need can literally print the path */
          || (strlen(string_value) > 1 && string_value[0] == '.' && string_value[1] == '/'))
            NixXML_print_value_nix(file, value, indent_level, userdata);
        else if(strcmp(string_value, "") == 0 || strcmp(string_value, ".") == 0) /* An empty string or . is considered current directory that requires a dedicated construct */
            fprintf(file, "./.");
        else
        {
            /* Otherwise, we consider the path a relative path */
            fprintf(file, "./");
            NixXML_print_value_nix(file, value, indent_level, userdata);
        }
    }
}

void NixXML_print_int_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    fprintf(file, "%d", *((int*)value));
}

void NixXML_print_store_path_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    fprintf(file, "builtins.storePath %s", (const char*)value);
}

void NixXML_print_list_element_nix(FILE *file, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    NixXML_print_indentation(file, indent_level);
    print_value(file, value, indent_level, userdata);
    fprintf(file, "\n");
}

void NixXML_print_list_nix(FILE *file, const void *list, const int indent_level, void *userdata, NixXML_PrintMembersFunc print_list_elements, NixXML_PrintValueFunc print_value)
{
    fprintf(file, "[\n");
    print_list_elements(file, list, indent_level >= 0 ? indent_level + 1 : indent_level, userdata, print_value);
    NixXML_print_indentation(file, indent_level);
    fprintf(file, "]");
}

void NixXML_print_attribute_nix(FILE *file, const char *name, const void *value, const int indent_level, void *userdata, NixXML_PrintValueFunc print_value)
{
    NixXML_print_indentation(file, indent_level);
    NixXML_print_string_nix(file, name, indent_level, userdata);
    fprintf(file, " = ");
    print_value(file, value, indent_level, userdata);
    fprintf(file, ";\n");
}

void NixXML_print_attrset_nix(FILE *file, const void *table, const int indent_level, void *userdata, NixXML_PrintMembersFunc print_attributes, NixXML_PrintValueFunc print_value)
{
    fprintf(file, "{\n");
    print_attributes(file, table, indent_level >= 0 ? indent_level + 1 : indent_level, userdata, print_value);
    NixXML_print_indentation(file, indent_level);
    fprintf(file, "}");
}
