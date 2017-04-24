/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'fault_block_layer.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/int_vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/vector.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fault_block_layer.h>
#include <ert/ecl/layer.h>

#define FAULT_BLOCK_LAYER_ID 2297476


/*
   The fault_block object is implemented as a separate object type in
   the fault_block.c file; however the fault blocks should be closely
   linked to the layer object in the fault_block_layer structure - it
   is therefor not possible/legal to create a fault block instance by
   itself. To support that encapsulation the fault_block.c file is
   included here, and the functions:

     fault_block_alloc();
     fault_block_free();

   Which must be called through the fault_block_layer class are made
   static.
*/

static fault_block_type * fault_block_alloc( const fault_block_layer_type * parent_layer , int block_id );
static void               fault_block_free( fault_block_type * block );

#include "fault_block.c"


struct fault_block_layer_struct {
  UTIL_TYPE_ID_DECLARATION;
  const ecl_grid_type * grid;
  int_vector_type     * block_map;
  layer_type          * layer;
  int                   k;
  vector_type         * blocks;
};


UTIL_IS_INSTANCE_FUNCTION(fault_block_layer , FAULT_BLOCK_LAYER_ID);
static UTIL_SAFE_CAST_FUNCTION(fault_block_layer , FAULT_BLOCK_LAYER_ID);


fault_block_type * fault_block_layer_add_block( fault_block_layer_type * layer , int block_id) {
  if (int_vector_safe_iget( layer->block_map , block_id) < 0) {
    fault_block_type * block = fault_block_alloc( layer , block_id );
    int storage_index = vector_get_size( layer->blocks );

    int_vector_iset( layer->block_map , block_id , storage_index );
    vector_append_owned_ref( layer->blocks , block , fault_block_free__ );

    return block;
  } else
    return NULL;
}





void fault_block_layer_scan_layer( fault_block_layer_type * fault_layer , layer_type * layer) {
  int i,j;
  int_vector_type * i_list = int_vector_alloc(0,0);
  int_vector_type * j_list = int_vector_alloc(0,0);

  for (j = 0; j < layer_get_ny( layer ); j++) {
    for (i = 0; i < layer_get_nx( layer); i++) {
      int cell_value = layer_iget_cell_value( layer , i , j );
      if (cell_value != 0) {
        layer_trace_block_content( layer , true , i , j , cell_value , i_list , j_list );
        {
          int c;
          int block_id = fault_block_layer_get_next_id( fault_layer );
          fault_block_type * fault_block = fault_block_layer_add_block( fault_layer , block_id );
          for (c=0; c < int_vector_size( i_list ); c++)
            fault_block_add_cell( fault_block , int_vector_iget( i_list , c ), int_vector_iget( j_list , c ));

        }
      }
    }
  }

  int_vector_free( i_list );
  int_vector_free( j_list );
}


/*
  Observe that the id values from the ecl_kw instance are not
  retained; the fault_block_layer instance gets new block id numbers,
  including a nonzero value for the cells which have value zero in the
  keyword.


   - The blocks in the fault_block_layer instance are guaranteed to be
     singly connected.

   - Observe that the numbering of the blocks is implicitly given by
     the spatial distribution of the values, and hence *completely
     random* with respect to the original values in the keyword.

*/


bool fault_block_layer_scan_kw( fault_block_layer_type * layer , const ecl_kw_type * fault_block_kw) {
  bool assign_zero = true;

  if (ecl_kw_get_size( fault_block_kw) != ecl_grid_get_global_size(layer->grid))
    return false;
  else if (!ecl_type_is_int(ecl_kw_get_data_type( fault_block_kw )))
    return false;
  else {
    int i,j;
    int max_block_id = 0;
    layer_type * work_layer = layer_alloc( ecl_grid_get_nx( layer->grid ) , ecl_grid_get_ny( layer->grid ));

    for (j=0; j < ecl_grid_get_ny( layer->grid ); j++) {
      for (i=0; i < ecl_grid_get_nx( layer->grid ); i++) {
        int g = ecl_grid_get_global_index3( layer->grid , i , j , layer->k );
        int block_id = ecl_kw_iget_int( fault_block_kw , g );


        if (block_id > 0) {
          layer_iset_cell_value( work_layer , i , j , block_id );
          max_block_id = util_int_max( block_id , max_block_id );
        }
      }
    }

    if (assign_zero)
      layer_replace_cell_values( work_layer , 0 , max_block_id + 1);

    fault_block_layer_scan_layer( layer , work_layer );
    layer_free( work_layer );
    return true;
  }
}



/**
   This function will just load the fault block distribution from
   fault_block_kw; it will not do any reordering or assign block ids
   to the regions with ID == 0.
*/


bool fault_block_layer_load_kw( fault_block_layer_type * layer , const ecl_kw_type * fault_block_kw) {
  if (ecl_kw_get_size( fault_block_kw) != ecl_grid_get_global_size(layer->grid))
    return false;
  else if (!ecl_type_is_int(ecl_kw_get_data_type( fault_block_kw )))
    return false;
  else {
    int i,j;

    for (j=0; j < ecl_grid_get_ny( layer->grid ); j++) {
      for (i=0; i < ecl_grid_get_nx( layer->grid ); i++) {
        int g = ecl_grid_get_global_index3( layer->grid , i , j , layer->k );
        int block_id = ecl_kw_iget_int( fault_block_kw , g );
        if (block_id > 0) {
          fault_block_layer_add_block( layer , block_id );
          {
            fault_block_type * fault_block = fault_block_layer_get_block( layer , block_id );
            fault_block_add_cell( fault_block , i,j );
          }
        }
      }
    }

    return true;
  }
}






fault_block_layer_type * fault_block_layer_alloc( const ecl_grid_type * grid , int k) {
  if ((k < 0) || (k >= ecl_grid_get_nz( grid )))
    return NULL;
  else {
    fault_block_layer_type * layer = util_malloc( sizeof * layer );
    UTIL_TYPE_ID_INIT( layer , FAULT_BLOCK_LAYER_ID);
    layer->grid = grid;
    layer->k    = k;
    layer->block_map = int_vector_alloc( 0 , -1);
    layer->blocks = vector_alloc_new();
    layer->layer = layer_alloc(ecl_grid_get_nx(grid) , ecl_grid_get_ny(grid));

    return layer;
  }
}


fault_block_type * fault_block_layer_iget_block( const fault_block_layer_type * layer , int storage_index) {
  return vector_iget( layer->blocks , storage_index );
}


fault_block_type * fault_block_layer_get_block( const fault_block_layer_type * layer , int block_id) {
  int storage_index = int_vector_safe_iget( layer->block_map , block_id);
  if (storage_index < 0)
    return NULL;
  else
    return vector_iget( layer->blocks , storage_index );
}


fault_block_type * fault_block_layer_safe_get_block( fault_block_layer_type * layer , int block_id) {
  int storage_index = int_vector_safe_iget( layer->block_map , block_id);
  if (storage_index < 0)
    return fault_block_layer_add_block( layer , block_id );
  else
    return vector_iget( layer->blocks , storage_index );
}



void fault_block_layer_del_block( fault_block_layer_type * layer , int block_id) {
  int storage_index = int_vector_safe_iget( layer->block_map , block_id);
  if (storage_index >= 0) {

    int_vector_iset( layer->block_map , block_id , -1 );
    vector_idel( layer->blocks , storage_index );
    {
      int index;

      for (index = 0; index < int_vector_size( layer->block_map ); index++) {
        int current_storage_index = int_vector_iget( layer->block_map , index );
        if (current_storage_index > storage_index)
          int_vector_iset( layer->block_map ,index , current_storage_index - 1);
      }
    }
  }
}






bool fault_block_layer_has_block( const fault_block_layer_type * layer , int block_id) {
  if (int_vector_safe_iget( layer->block_map , block_id) >= 0)
    return true;
  else
    return false;
}


int fault_block_layer_get_max_id( const fault_block_layer_type * layer ) {
   return int_vector_size( layer->block_map ) - 1;
}

int fault_block_layer_get_next_id( const fault_block_layer_type * layer ) {
  if (int_vector_size( layer->block_map ) == 0)
    return 1;
  else
    return int_vector_size( layer->block_map );
}

int fault_block_layer_get_size( const fault_block_layer_type * layer ) {
  return vector_get_size( layer->blocks );
}


int fault_block_layer_get_k( const fault_block_layer_type * layer ) {
  return layer->k;
}



void fault_block_layer_free( fault_block_layer_type * layer ) {
  int_vector_free( layer->block_map );
  vector_free( layer->blocks );
  layer_free( layer->layer );
  free(layer);
}


void fault_block_layer_free__( void * arg ) {
  fault_block_layer_type * layer = fault_block_layer_safe_cast( arg );
  fault_block_layer_free( layer );
}


void fault_block_layer_insert_block_content( fault_block_layer_type * layer , const fault_block_type * src_block) {
  int next_block_id = fault_block_layer_get_next_id( layer );
  fault_block_type * target_block = fault_block_layer_add_block( layer , next_block_id );
  fault_block_copy_content( target_block , src_block );
}



bool fault_block_layer_export( const fault_block_layer_type * layer , ecl_kw_type * faultblock_kw) {
  if (ecl_type_is_int(ecl_kw_get_data_type( faultblock_kw )) && (ecl_kw_get_size( faultblock_kw ) == ecl_grid_get_global_size( layer->grid ))) {
    int i,j;

    for (j=0; j < ecl_grid_get_ny( layer->grid ); j++) {
      for (i=0; i < ecl_grid_get_nx( layer->grid ); i++) {
        int g = ecl_grid_get_global_index3( layer->grid , i, j , layer->k );
        int cell_value = layer_iget_cell_value( layer->layer , i , j );
        ecl_kw_iset_int( faultblock_kw , g , cell_value);
      }
    }
    return true;
  } else
    return false;
}


const ecl_grid_type * fault_block_layer_get_grid( const fault_block_layer_type * layer ) {
  return layer->grid;
}


layer_type * fault_block_layer_get_layer( const fault_block_layer_type * layer ) {
  return layer->layer;
}
