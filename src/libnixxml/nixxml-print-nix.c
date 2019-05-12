/*
 * Copyright (c) 2019 Sander van der Burg
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

void NixXML_print_int_nix(FILE *file, const void *value, const int indent_level, void *userdata)
{
    fprintf(file, "%d", *((int*)value));
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
