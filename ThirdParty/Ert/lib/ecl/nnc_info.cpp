/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'nnc_info.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.hpp>
#include <ert/util/vector.hpp>  
#include <ert/util/type_macros.hpp>

#include <ert/ecl/nnc_info.hpp>
#include <ert/ecl/nnc_vector.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>

#define NNC_INFO_TYPE_ID 675415078


struct nnc_info_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type     * lgr_list;       /*List of int vector * for nnc connections for LGRs*/
  int_vector_type * lgr_index_map; /* A vector that maps LGR-nr to index into the LGR_list.*/
  int               lgr_nr;        /* The lgr_nr of the cell holding this nnc_info structure. */ 
}; 

static void nnc_info_add_vector( nnc_info_type * nnc_info , nnc_vector_type * nnc_vector);
UTIL_IS_INSTANCE_FUNCTION( nnc_info , NNC_INFO_TYPE_ID )


nnc_info_type * nnc_info_alloc(int lgr_nr) {
  nnc_info_type * nnc_info = (nnc_info_type*)util_malloc( sizeof * nnc_info );
  UTIL_TYPE_ID_INIT(nnc_info , NNC_INFO_TYPE_ID);
  nnc_info->lgr_list = vector_alloc_new(); 
  nnc_info->lgr_index_map = int_vector_alloc(0, -1); 
  nnc_info->lgr_nr = lgr_nr;
  return nnc_info; 
}



nnc_info_type * nnc_info_alloc_copy( const nnc_info_type * src_info ) {
  nnc_info_type * copy_info = nnc_info_alloc( src_info->lgr_nr );
  int ivec;
  
  for (ivec = 0; ivec < vector_get_size( src_info->lgr_list ); ivec++) {
    nnc_vector_type * copy_vector = nnc_vector_alloc_copy( (const nnc_vector_type*)vector_iget_const( src_info->lgr_list , ivec));
    nnc_info_add_vector( copy_info , copy_vector );
  }

  return copy_info;
}


bool nnc_info_equal( const nnc_info_type * nnc_info1 , const nnc_info_type * nnc_info2 ) {
  if (nnc_info1 == nnc_info2)
    return true;
  
  if ((nnc_info1 == NULL) || (nnc_info2 == NULL))
    return false;

  {
    if (nnc_info1->lgr_nr != nnc_info2->lgr_nr)
      return false;
    
    if ((int_vector_size( nnc_info1->lgr_index_map ) > 0) && (int_vector_size( nnc_info2->lgr_index_map ) > 0)) {
      int max_lgr_nr = util_int_max( int_vector_size( nnc_info1->lgr_index_map ), 
                                     int_vector_size( nnc_info2->lgr_index_map ) );
      int lgr_nr = 0;
      
      while (true) {
        nnc_vector_type * vector1 = nnc_info_get_vector( nnc_info1 , lgr_nr );
        nnc_vector_type * vector2 = nnc_info_get_vector( nnc_info2 , lgr_nr );
        
        if (!nnc_vector_equal(vector1 , vector2))
          return false;
        
        lgr_nr++;
        if (lgr_nr > max_lgr_nr)
          return true;
      } 
    } else {
      if (int_vector_size( nnc_info1->lgr_index_map ) == int_vector_size( nnc_info2->lgr_index_map ))
        return true;
      else
        return false;
    }
  }
}


void nnc_info_free( nnc_info_type * nnc_info ) {
  vector_free(nnc_info->lgr_list); 
  int_vector_free(nnc_info->lgr_index_map); 
  free (nnc_info); 
}

nnc_vector_type * nnc_info_get_vector( const nnc_info_type * nnc_info , int lgr_nr) {
  int lgr_index = int_vector_safe_iget( nnc_info->lgr_index_map , lgr_nr );
  if (-1 == lgr_index)
    return NULL;
  else
    return (nnc_vector_type*)vector_iget( nnc_info->lgr_list , lgr_index );
}


nnc_vector_type * nnc_info_iget_vector( const nnc_info_type * nnc_info , int lgr_index) {
  return (nnc_vector_type*)vector_iget( nnc_info->lgr_list , lgr_index );
}


nnc_vector_type * nnc_info_get_self_vector( const nnc_info_type * nnc_info ) {
  return nnc_info_get_vector( nnc_info , nnc_info->lgr_nr );
}

static void nnc_info_add_vector( nnc_info_type * nnc_info , nnc_vector_type * nnc_vector) {
  vector_append_owned_ref( nnc_info->lgr_list , nnc_vector , nnc_vector_free__ );
  int_vector_iset( nnc_info->lgr_index_map , nnc_vector_get_lgr_nr( nnc_vector ) , vector_get_size( nnc_info->lgr_list ) - 1 );
}


static void nnc_info_assert_vector( nnc_info_type * nnc_info , int lgr_nr ) {
  nnc_vector_type * nnc_vector = nnc_info_get_vector( nnc_info , lgr_nr);
  if (!nnc_vector) {
    nnc_vector = nnc_vector_alloc( lgr_nr );
    nnc_info_add_vector( nnc_info , nnc_vector );
  }
}



void nnc_info_add_nnc(nnc_info_type * nnc_info, int lgr_nr, int global_cell_number, int nnc_index) {
  nnc_info_assert_vector( nnc_info , lgr_nr );
  {
    nnc_vector_type * nnc_vector = nnc_info_get_vector( nnc_info , lgr_nr );
    nnc_vector_add_nnc( nnc_vector , global_cell_number , nnc_index);
  }
}
   

const int_vector_type * nnc_info_get_grid_index_list(const nnc_info_type * nnc_info, int lgr_nr) { 
  nnc_vector_type * nnc_vector = nnc_info_get_vector( nnc_info , lgr_nr );
  if (nnc_vector)
    return nnc_vector_get_grid_index_list( nnc_vector );
  else
    return NULL;
}


const int_vector_type * nnc_info_iget_grid_index_list(const nnc_info_type * nnc_info, int lgr_index) { 
  nnc_vector_type * nnc_vector = nnc_info_iget_vector( nnc_info , lgr_index );
  if (nnc_vector)
    return nnc_vector_get_grid_index_list( nnc_vector );
  else
    return NULL;
}



const int_vector_type * nnc_info_get_self_grid_index_list(const nnc_info_type * nnc_info) { 
  return nnc_info_get_grid_index_list( nnc_info , nnc_info->lgr_nr );
}



int nnc_info_get_lgr_nr( const nnc_info_type * nnc_info ) {
  return nnc_info->lgr_nr;
}


int nnc_info_get_size( const nnc_info_type * nnc_info ) {
  return vector_get_size( nnc_info->lgr_list );
}


int nnc_info_get_total_size( const nnc_info_type * nnc_info ) {
  int num_nnc = 0;
  int ivec;
  for (ivec = 0; ivec < vector_get_size( nnc_info->lgr_list ); ivec++) {
    const nnc_vector_type * nnc_vector = (const nnc_vector_type*)vector_iget( nnc_info->lgr_list , ivec );
    num_nnc += nnc_vector_get_size( nnc_vector );
  }
  return num_nnc;
}


void nnc_info_fprintf(const nnc_info_type * nnc_info , FILE * stream) {
  fprintf(stream,"LGR_NR:%d \n",nnc_info->lgr_nr);
  {
    int lgr_nr;
    for (lgr_nr=0; lgr_nr < int_vector_size( nnc_info->lgr_index_map ); lgr_nr++) {
      int lgr_index = int_vector_iget( nnc_info->lgr_index_map , lgr_nr );
      if (lgr_index >= 0) {
        printf("   %02d -> %02d  => ",lgr_nr , lgr_index);
        {
          const int_vector_type * index_list = nnc_info_iget_grid_index_list( nnc_info , lgr_index );
          int_vector_fprintf( index_list , stream , " " , "%d");
          printf("\n");
        }
      }
    }
  }
  fprintf(stream , "\n");
}
