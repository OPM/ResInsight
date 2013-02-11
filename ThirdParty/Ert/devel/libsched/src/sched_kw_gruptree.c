/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_gruptree.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h>

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

#include <ert/sched/sched_kw_gruptree.h>
#include <ert/sched/sched_util.h>
#include <ert/sched/sched_macros.h>


struct sched_kw_gruptree_struct
{
  hash_type * gruptree_hash;  /* The use of hash implies that the ordering within one GRUPTREE instance is not retained. */
};



/***********************************************************************/



static void sched_kw_gruptree_add_well(sched_kw_gruptree_type * kw , const char * child_group , const char * parent_group) {
  hash_insert_string(kw->gruptree_hash, child_group , parent_group);
}


static sched_kw_gruptree_type * sched_kw_gruptree_alloc_empty()
{
  sched_kw_gruptree_type * kw = util_malloc(sizeof * kw);
  kw->gruptree_hash = hash_alloc();
  
  return kw;
};



/***********************************************************************/


sched_kw_gruptree_type * sched_kw_gruptree_alloc(const stringlist_type * tokens , int * token_index ) {
  sched_kw_gruptree_type * kw = sched_kw_gruptree_alloc_empty();
  int eokw                    = false;
  do {
    stringlist_type * line_tokens = sched_util_alloc_line_tokens( tokens , false , 0 , token_index );
    if (line_tokens == NULL)
      eokw = true;
    else {
      const char * parent_group = "FIELD";
      const char * child_group  =  stringlist_iget( line_tokens , 0 );
      if (stringlist_get_size( line_tokens ) == 2) 
        parent_group = stringlist_iget( line_tokens , 1 );
      
      sched_kw_gruptree_add_well(kw , child_group , parent_group );
      
      stringlist_free( line_tokens );
    } 
    
  } while (!eokw);
  return kw;
}



void sched_kw_gruptree_free(sched_kw_gruptree_type * kw)
{
  hash_free(kw->gruptree_hash);
  free(kw);
};


void sched_kw_gruptree_fprintf(const sched_kw_gruptree_type * kw, FILE * stream)
{

  fprintf(stream, "GRUPTREE\n");
  {
    const int   num_keys = hash_get_size(kw->gruptree_hash);
    char ** child_list   = hash_alloc_keylist(kw->gruptree_hash);
    int i;

    for (i = 0; i < num_keys; i++) {
      const char * parent_name = hash_get_string(kw->gruptree_hash , child_list[i]);
      fprintf(stream,"  '%s'  '%s' /\n",child_list[i] , parent_name);
    }
    util_free_stringlist( child_list , num_keys );
  }
  fprintf(stream,"/\n\n");
};




void sched_kw_gruptree_init_child_parent_list( const sched_kw_gruptree_type * kw , stringlist_type * child , stringlist_type * parent) {
  stringlist_clear( child );
  stringlist_clear( parent );
  {
    hash_iter_type * iter = hash_iter_alloc( kw->gruptree_hash );
    while (!hash_iter_is_complete( iter )) {
      const char * child_group  = hash_iter_get_next_key( iter );
      const char * parent_group = hash_get_string( kw->gruptree_hash , child_group );
      
      stringlist_append_copy( child , child_group );       /* <- The iterator keys go out of scope when hash_iter_free() is called. */
      stringlist_append_ref( parent , parent_group );
    }
    hash_iter_free( iter );
  }
}




void sched_kw_gruptree_alloc_child_parent_list(const sched_kw_gruptree_type * kw, char *** __children, char *** __parents, int * num_pairs)
{
  *num_pairs = hash_get_size(kw->gruptree_hash);
  char ** children = hash_alloc_keylist(kw->gruptree_hash);
  char ** parents  = util_malloc(*num_pairs * sizeof * parents);

  for(int child_nr = 0; child_nr < *num_pairs; child_nr++)
  {
    parents[child_nr] = util_alloc_string_copy(hash_get_string(kw->gruptree_hash, children[child_nr]));
  }

  *__children = children;
  *__parents  = parents;
}



static sched_kw_gruptree_type * sched_kw_gruptree_copyc(const sched_kw_gruptree_type * src) {
  sched_kw_gruptree_type * target = sched_kw_gruptree_alloc_empty();
  hash_iter_type * iter = hash_iter_alloc(src->gruptree_hash);
  const char * kw = hash_iter_get_next_key(iter);
  while (kw != NULL) {
    char * parent_name = hash_get_string(src->gruptree_hash , kw);
    hash_insert_string( target->gruptree_hash , kw , parent_name);
    kw = hash_iter_get_next_key(iter);
  }
  hash_iter_free(iter);
  return target;
}



/***********************************************************************/

KW_IMPL(gruptree)
