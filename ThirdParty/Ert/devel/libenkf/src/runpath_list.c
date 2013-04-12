/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'runpath_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <pthread.h>

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/runpath_list.h>

typedef struct runpath_node_struct runpath_node_type;


struct runpath_list_struct {
  pthread_rwlock_t   lock;
  vector_type      * list;
  char             * line_fmt;   // Format string : Values are in the order: (iens , runpath , basename)
};


#define RUNPATH_NODE_TYPE_ID  661400541
struct runpath_node_struct {
  UTIL_TYPE_ID_DECLARATION;
  int    iens;
  char * runpath;
  char * basename;
};


/*****************************************************************/

  UTIL_SAFE_CAST_FUNCTION( runpath_node , RUNPATH_NODE_TYPE_ID )
  UTIL_SAFE_CAST_FUNCTION_CONST( runpath_node , RUNPATH_NODE_TYPE_ID )

  static runpath_node_type * runpath_node_alloc( int iens, const char * runpath , const char * basename) {
    runpath_node_type * node = util_malloc( sizeof * node );
    UTIL_TYPE_ID_INIT( node , RUNPATH_NODE_TYPE_ID );

    node->iens = iens;
    node->runpath = util_alloc_string_copy( runpath );
    node->basename = util_alloc_string_copy( basename );

    return node;
  }


static void runpath_node_free( runpath_node_type * node ) {
  free(node->basename);
  free(node->runpath);
  free(node);
}


static void runpath_node_free__( void * arg ) {
  runpath_node_type * node = runpath_node_safe_cast( arg );
  runpath_node_free( node );
}

static int runpath_node_cmp( const void * arg1 , const void * arg2) {
  const runpath_node_type * node1 = runpath_node_safe_cast_const( arg1 );
  const runpath_node_type * node2 = runpath_node_safe_cast_const( arg2 );
  {
    if (node1->iens > node2->iens)
      return 1;
    else if (node1->iens < node2->iens)
      return -1;
    else
      return 0;
  }
}


static void runpath_node_fprintf( const runpath_node_type * node , const char * line_fmt , FILE * stream) {
  fprintf(stream , line_fmt , node->iens, node->runpath , node->basename);
}


/*****************************************************************/


runpath_list_type * runpath_list_alloc() {
  runpath_list_type * list = util_malloc( sizeof * list );
  list->list     = vector_alloc_new();
  list->line_fmt = NULL;
  pthread_rwlock_init( &list->lock , NULL );
  return list;
}


void runpath_list_free( runpath_list_type * list ) {
  vector_free( list->list );
  util_safe_free( list->line_fmt );
  free( list );
}


int runpath_list_size( const runpath_list_type * list ) {
  return vector_get_size( list->list );
}


void runpath_list_sort( runpath_list_type * list ) {
  pthread_rwlock_wrlock( &list->lock );
  {
    vector_sort( list->list , runpath_node_cmp );
  }
  pthread_rwlock_unlock( &list->lock );
}


void runpath_list_add( runpath_list_type * list , int iens , const char * runpath , const char * basename) {
  runpath_node_type * node = runpath_node_alloc( iens , runpath , basename );
  pthread_rwlock_wrlock( &list->lock );
  {
    vector_append_owned_ref( list->list , node , runpath_node_free__ );
  }
  pthread_rwlock_unlock( &list->lock );
}


void runpath_list_clear( runpath_list_type * list ) {
  pthread_rwlock_wrlock( &list->lock );
  {
    vector_clear( list->list );
  }
  pthread_rwlock_unlock( &list->lock );
}

/*****************************************************************/

void runpath_list_set_line_fmt( runpath_list_type * list , const char * line_fmt ) {
  list->line_fmt = util_realloc_string_copy( list->line_fmt , line_fmt );
}


const char * runpath_list_get_line_fmt( const runpath_list_type * list ) {
  if (list->line_fmt == NULL)
    return RUNPATH_LIST_DEFAULT_LINE_FMT;
  else
    return list->line_fmt;
}

/*****************************************************************/


static const runpath_node_type * runpath_list_iget_node__( const runpath_list_type * list , int index) {
  return vector_iget_const( list->list , index );
}



static const runpath_node_type * runpath_list_iget_node( runpath_list_type * list , int index) {
  const runpath_node_type * node;
  {
    pthread_rwlock_rdlock( &list->lock );
    node = runpath_list_iget_node__( list , index );
    pthread_rwlock_unlock( &list->lock );
  }
  return node;
}


int runpath_list_iget_iens( runpath_list_type * list , int index) {
  const runpath_node_type * node = runpath_list_iget_node( list , index );
  return node->iens;
}


void runpath_list_fprintf(runpath_list_type * list , FILE * stream) {
  pthread_rwlock_rdlock( &list->lock );
  {
    const char * line_fmt = runpath_list_get_line_fmt( list );
    int index;
    for (index =0; index < vector_get_size( list->list ); index++) {
      const runpath_node_type * node = runpath_list_iget_node__( list , index );
      runpath_node_fprintf( node , line_fmt , stream );
    }
  }
  pthread_rwlock_unlock( &list->lock );
}
