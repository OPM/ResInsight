/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'xpath1.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)


static void usage(const char *name);
static int example(const char *filename);
static int setNodeValue(const xmlXPathContextPtr xc, const xmlChar * xpath, const xmlChar * value);
static void update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar * value);


int main(int argc, char **argv) {
    /* Parse command line and process file */
    if (argc != 2) {
        fprintf(stderr, "Error: wrong number of arguments.\n");
        usage(argv[0]);
        return(-1);
    }

    /* Init libxml */
    xmlInitParser();
    LIBXML_TEST_VERSION

    /* Do the main job */
    if (example(argv[1])) {
        usage(argv[0]);
        return(-1);
    }

    /* Shutdown libxml */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
    return 0;
}

/**
 * usage:
 * @name:		the program name.
 *
 * Prints usage information.
 */
static void usage(const char *name) {
    assert(name);

    fprintf(stderr, "Usage: %s <xml-file> \n", name);
}

/**
 * example:
 * @filename:		the input XML filename.
 *
 * Parses input XML file, evaluates XPath expression and update the nodes
 * then print the result.
 *
 * Returns 0 on success and a negative value otherwise.
 */
static int example(const char* filename) {
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx;

    assert(filename);

    /* Load XML document */
    doc = xmlParseFile(filename);
    if (doc == NULL) {
        fprintf(stderr, "Error: unable to parse file \"%s\"\n", filename);
        return(-1);
    }

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        xmlFreeDoc(doc);
        return(-1);
    }


    //change the PATH attribute of GRID
     if(setNodeValue(xpathCtx, "//ert/eclipse/grid/@path", "sillypath")) {
        xmlFreeDoc(doc);
        return -1;
     }

    //node TEST does not exist but will not create an error. Prints "no nodes!"
     if(setNodeValue(xpathCtx, "//ert/eclipse/test", "sillypath")) {
        xmlFreeDoc(doc);
        return -1;
     }





    /* Cleanup of XPath data */
    xmlXPathFreeContext(xpathCtx);

    /* dump the resulting document */
    xmlDocDump(stdout, doc);


    /* free the document */
    xmlFreeDoc(doc);

    return(0);
}

static int setNodeValue(const xmlXPathContextPtr xc, const xmlChar* xpath, const xmlChar* value) {
    xmlXPathObjectPtr xpathObj;
    
    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(xpath, xc);
    if(xpathObj == NULL) {
        fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpath);
        xmlXPathFreeContext(xc);
        return(-1);
    }

    if(xpathObj->nodesetval->nodeNr == 0) {
        printf("No nodes!\n");
    }
    /* update selected nodes */
    update_xpath_nodes(xpathObj->nodesetval, value);
    xmlXPathFreeObject(xpathObj);
    return 0;
}

/**
 * update_xpath_nodes:
 * @nodes:		the nodes set.
 * @value:		the new value for the node(s)
 *
 * Prints the @nodes content to @output.
 */
static void update_xpath_nodes(xmlNodeSetPtr nodes, const xmlChar* value) {
    int size;
    int i;

    assert(value);
    size = (nodes) ? nodes->nodeNr : 0;

    /*
     * NOTE: the nodes are processed in reverse order, i.e. reverse document
     *       order because xmlNodeSetContent can actually free up descendant
     *       of the node and such nodes may have been selected too ! Handling
     *       in reverse order ensure that descendant are accessed first, before
     *       they get removed. Mixing XPath and modifications on a tree must be
     *       done carefully !
     */
    for(i = size - 1; i >= 0; i--) {
        assert(nodes->nodeTab[i]);

        xmlNodeSetContent(nodes->nodeTab[i], value);
        /*
         * All the elements returned by an XPath query are pointers to
         * elements from the tree *except* namespace nodes where the XPath
         * semantic is different from the implementation in libxml2 tree.
         * As a result when a returned node set is freed when
         * xmlXPathFreeObject() is called, that routine must check the
         * element type. But node from the returned set may have been removed
         * by xmlNodeSetContent() resulting in access to freed data.
         * This can be exercised by running
         *       valgrind xpath2 test3.xml '//discarded' discarded
         * There is 2 ways around it:
         *   - make a copy of the pointers to the nodes from the result set
         *     then call xmlXPathFreeObject() and then modify the nodes
         * or
         *   - remove the reference to the modified nodes from the node set
         *     as they are processed, if they are not namespace nodes.
         */
        if (nodes->nodeTab[i]->type != XML_NAMESPACE_DECL)
            nodes->nodeTab[i] = NULL;
    }
}

#else
int main(void) {
    fprintf(stderr, "XPath support not compiled in\n");
    exit(1);
}
#endif