/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'tree1.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

/**
 * section: Tree
 * synopsis: Navigates a tree to print element names
 * purpose: Parse a file to a tree, use xmlDocGetRootElement() to
 *          get the root element, then walk the document and print
 *          all the element name in document order.
 * usage: tree1 filename_or_URL
 * test: tree1 test2.xml > tree1.tmp ; diff tree1.tmp tree1.res ; rm tree1.tmp
 * author: Dodji Seketeli
 * copy: see Copyright for the status of this software.
 */
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifdef LIBXML_TREE_ENABLED

/*
 *To compile this file using gcc you can type
 *gcc `xml2-config --cflags --libs` -o xmlexample libxml2-example.c
 */

/**
 * print_element_names:
 * @a_node: the initial xml node to consider.
 *
 * Prints the names of the all the xml elements
 * that are siblings or children of a given xml node.
 */
static void print_element_names(xmlNode * a_node, char *str) {
    xmlNode *cur_node = NULL;
    char level[100];
    sprintf(level, "%s-", str);
    //strcat(level, str);

    for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            printf("%s node type: Element, name: %s\n", level, cur_node->name);
        }

        print_element_names(cur_node->children, level);
    }
}


/**
 * Simple example to parse a fi le called "file.xml",
 * walk down the DOM, and print the name of the
 * xml elements nodes.
 */
int main(int argc, char **argv) {
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    if (argc != 2)
        return(1);

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    /*parse the file and get the DOM */
    doc = xmlReadFile(argv[1], NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", argv[1]);
    }

    /*Get the root element node */
    root_element = xmlDocGetRootElement(doc);

    print_element_names(root_element, "");

    /*free the document */
    xmlFreeDoc(doc);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return 0;
}
#else
int main(void) {
    fprintf(stderr, "Tree support not compiled in\n");
    exit(1);
}
#endif
