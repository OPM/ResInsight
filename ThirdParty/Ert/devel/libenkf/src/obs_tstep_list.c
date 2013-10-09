/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'obs_tstep_list.c'
    
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

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/int_vector.h>

#include <ert/enkf/obs_tstep_list.h>


#define OBS_TSTEP_LIST_TYPE_ID 84865209

struct obs_tstep_list_struct {
   UTIL_TYPE_ID_DECLARATION;
   bool all_active;
   int_vector_type * tstep_list;
};


UTIL_IS_INSTANCE_FUNCTION( obs_tstep_list , OBS_TSTEP_LIST_TYPE_ID )


obs_tstep_list_type * obs_tstep_list_alloc() {
  obs_tstep_list_type * list = util_malloc( sizeof * list );
  UTIL_TYPE_ID_INIT( list , OBS_TSTEP_LIST_TYPE_ID );
  list->all_active = true;
  list->tstep_list = int_vector_alloc(0,0);
  return list;
}


void obs_tstep_list_free( obs_tstep_list_type * list ) {
  free( list );
}



bool obs_tstep_list_all_active( const obs_tstep_list_type * list ) {
   return list->all_active;
}


bool obs_tstep_list_contains( const obs_tstep_list_type * list , int tstep) {
  if (int_vector_index_sorted( list->tstep_list , tstep) == -1) 
    return false;
  else
    return true;
}



void obs_tstep_list_add_tstep( obs_tstep_list_type * list , int tstep) {
  if (!obs_tstep_list_contains( list , tstep)) {
      if (int_vector_size( list->tstep_list )) {
          int last  = int_vector_get_last( list->tstep_list );
          int_vector_append( list->tstep_list , tstep ); 
          if (tstep < last)
             int_vector_sort( list->tstep_list); 
      } else
         int_vector_append( list->tstep_list , tstep ); 
   }  
   list->all_active = false;
}


void obs_tstep_list_add_range( obs_tstep_list_type * list , int step1 , int step2) {
  int tstep;
  for (tstep = step1; tstep <= step2; tstep++)
    obs_tstep_list_add_tstep( list , tstep );
}


int obs_tstep_list_get_size( const obs_tstep_list_type * list ) {
   return int_vector_size( list->tstep_list );
}


int obs_tstep_list_iget( const obs_tstep_list_type * list , int index) {
   return int_vector_iget( list->tstep_list , index);
}


int obs_tstep_list_get_last( const obs_tstep_list_type * list ) {
   return int_vector_get_last( list->tstep_list );
}
