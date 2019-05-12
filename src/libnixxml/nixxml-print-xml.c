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

#include "nixxml-print-xml.h"

static void print_string_xml(FILE *file, const char *string_value)
{
    unsigned int i = 0;

    while(string_value[i] != '\0')
    {
        char c = string_value[i];

        switch(c)
        {
            case '<':
                fprintf(file, "&lt;");
                break;
            case '>':
                fprintf(file, "&gt;");
                break;
            default:
                fputc(c, file);
        }

        i++;
    }
}

void NixXML_print_open_root_tag(FILE *file, const char *root_element_name)
{
    fprintf(file, "<%s", root_element_name);
}

void NixXML_print_open_verbose_attr_tag(FILE *file, const char *root_element_name, const char *name_property_name, const char *name)
{
    fprintf(file, "<%s %s=\"", root_element_name, name_property_name);
    print_string_xml(file, name);
    fprintf(file, "\"");
}

void NixXML_print_close_root_tag(FILE *file, const char *root_element_name)
{
    fprintf(file, "</%s>", root_element_name);
}

static void print_type_suffix(FILE *file, const char *type_property_name, const char *type)
{
    if(type_property_name != NULL)
        fprintf(file, " %s=\"%s\"", type_property_name, type);

    fputc('>', file);
}

void NixXML_print_string_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    print_type_suffix(file, type_property_name, "string");
    print_string_xml(file, value);
}

void NixXML_print_string_as_int_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    print_type_suffix(file, type_property_name, "int");
    print_string_xml(file, value);
}

void NixXML_print_string_as_float_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    print_type_suffix(file, type_property_name, "int");
    print_string_xml(file, value);
}

void NixXML_print_string_as_bool_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    print_type_suffix(file, type_property_name, "bool");
    print_string_xml(file, value);
}

void NixXML_print_int_xml(FILE *file, const void *value, const int indent_level, const char *type_property_name, void *userdata)
{
    print_type_suffix(file, type_property_name, "int");
    fprintf(file, "%d", *((int*)value));
}

void NixXML_print_list_element_xml(FILE *file, const char *child_element_name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    fprintf(file, "\n");
    NixXML_print_indentation(file, indent_level);
    NixXML_print_open_root_tag(file, child_element_name);
    print_value(file, value, indent_level, type_property_name, userdata);
    NixXML_print_close_root_tag(file, child_element_name);
}

void NixXML_print_list_xml(FILE *file, const void *list, const char *child_element_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLListMembersFunc print_list_elements, NixXML_PrintXMLValueFunc print_value)
{
    print_type_suffix(file, type_property_name, "list");
    print_list_elements(file, child_element_name, list, indent_level >= 0 ? indent_level + 1 : indent_level, type_property_name, userdata, print_value);
    fprintf(file, "\n");
    NixXML_print_indentation(file, indent_level);
}

void NixXML_print_simple_attribute_xml(FILE *file, const char *name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    fprintf(file, "\n");
    NixXML_print_indentation(file, indent_level);
    NixXML_print_open_root_tag(file, name);
    print_value(file, value, indent_level, type_property_name, userdata);
    NixXML_print_close_root_tag(file, name);
    NixXML_print_indentation(file, indent_level);
}

void NixXML_print_verbose_attribute_xml(FILE *file, const char *child_element_name, const char *name_property_name, const char *name, const void *value, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintXMLValueFunc print_value)
{
    fprintf(file, "\n");
    NixXML_print_indentation(file, indent_level);
    NixXML_print_open_verbose_attr_tag(file, child_element_name, name_property_name, name);
    print_value(file, value, indent_level, type_property_name, userdata);
    NixXML_print_close_root_tag(file, child_element_name);
    NixXML_print_indentation(file, indent_level);
}

void NixXML_print_simple_attrset_xml(FILE *file, const void *table, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintSimpleXMLMembersFunc print_attributes, NixXML_PrintXMLValueFunc print_value)
{
    print_type_suffix(file, type_property_name, "attrs");
    print_attributes(file, table, indent_level >= 0 ? indent_level + 1 : indent_level, type_property_name, userdata, print_value);
    fprintf(file, "\n");
    NixXML_print_indentation(file, indent_level);
}

void NixXML_print_verbose_attrset_xml(FILE *file, const void *table, const char *child_element_name, const char *name_property_name, const int indent_level, const char *type_property_name, void *userdata, NixXML_PrintVerboseXMLMembersFunc print_attributes, NixXML_PrintXMLValueFunc print_value)
{
    print_type_suffix(file, type_property_name, "attrs");
    print_attributes(file, table, child_element_name, name_property_name, indent_level >= 0 ? indent_level + 1 : indent_level, type_property_name, userdata, print_value);
    fprintf(file, "\n");
    NixXML_print_indentation(file, indent_level);
}
