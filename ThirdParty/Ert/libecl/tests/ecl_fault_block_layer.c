/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ecl_fault_block_layer.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <unistd.h>

#include <ert/util/test_util.h>

#include <ert/geometry/geo_polygon_collection.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fault_block_layer.h>
#include <ert/ecl/ecl_type.h>




void test_create( const ecl_grid_type * grid , ecl_kw_type * fault_block_kw) {
  int k = 0;
  int i,j;
  
  for (j=0; j < ecl_grid_get_ny( grid ); j++) {
    for (i = 0; i < ecl_grid_get_nx( grid ); i++) {
      
      int g = ecl_grid_get_global_index3( grid , i,j,k);
      ecl_kw_iset_int( fault_block_kw , g , 9 );
    }
  }

  {
    fault_block_layer_type * layer = fault_block_layer_alloc( grid , k );
    test_assert_int_equal( 1 , fault_block_layer_get_next_id( layer ));
    fault_block_layer_scan_kw( layer , fault_block_kw);
    {
      fault_block_type * block = fault_block_layer_iget_block( layer , 0 );
      double x,y,z;
      ecl_grid_get_xyz3( grid , 4,4,k , &x, &y , &z );
      test_assert_double_equal( x , fault_block_get_xc( block ));
      test_assert_double_equal( y , fault_block_get_yc( block ));
    }
    
    fault_block_layer_free( layer );
  }
}




void test_create_invalid( const ecl_grid_type * grid ) {
  ecl_kw_type * fault_blk_kw = ecl_kw_alloc("FAULTBLK" , ecl_grid_get_global_size( grid ) - 1, ECL_INT);
  
  test_assert_NULL( fault_block_layer_alloc( grid , 7 ));
  
  ecl_kw_free( fault_blk_kw );
}


void test_trace_edge( const ecl_grid_type * grid) {
  const int k = 1;
  fault_block_layer_type * layer = fault_block_layer_alloc( grid , k );
  double_vector_type * x_list = double_vector_alloc( 0,0);
  double_vector_type * y_list = double_vector_alloc( 0,0);
  fault_block_type * block = fault_block_layer_safe_get_block( layer , 99);
  int_vector_type * cell_list = int_vector_alloc(0,0);
    
  test_assert_false( fault_block_trace_edge( block , x_list , y_list , cell_list));
  fault_block_add_cell( block , 0,0);
  test_assert_true( fault_block_trace_edge( block , x_list , y_list , cell_list));
  test_assert_int_equal( 4 , double_vector_size( x_list ));
  test_assert_int_equal( 4 , double_vector_size( y_list ));
  
  test_assert_double_equal( 0 , double_vector_iget( x_list , 0 ));
  test_assert_double_equal( 1 , double_vector_iget( x_list , 1 ));
  test_assert_double_equal( 1 , double_vector_iget( x_list , 2 ));
  test_assert_double_equal( 0 , double_vector_iget( x_list , 3 ));
  
  test_assert_double_equal( 0 , double_vector_iget( y_list , 0 ));
  test_assert_double_equal( 0 , double_vector_iget( y_list , 1 ));
  test_assert_double_equal( 1 , double_vector_iget( y_list , 2 ));
  test_assert_double_equal( 1 , double_vector_iget( y_list , 3 ));

  test_assert_int_equal( 1 , int_vector_size( cell_list ));
  test_assert_int_equal( 0 , int_vector_iget( cell_list , 0));

  int_vector_free( cell_list );
  double_vector_free( x_list );
  double_vector_free( y_list );
}


void test_export( const ecl_grid_type * grid) {
  fault_block_layer_type * layer = fault_block_layer_alloc( grid , 0 );
  ecl_kw_type * ecl_kw1 = ecl_kw_alloc("FAULTBLK" , ecl_grid_get_global_size( grid )     , ECL_INT);
  ecl_kw_type * ecl_kw2 = ecl_kw_alloc("FAULTBLK" , ecl_grid_get_global_size( grid ) + 1 , ECL_INT);
  ecl_kw_type * ecl_kw3 = ecl_kw_alloc("FAULTBLK" , ecl_grid_get_global_size( grid )     , ECL_FLOAT);
  fault_block_type * block = fault_block_layer_add_block( layer , 10 );
  
  fault_block_add_cell( block , 0 , 0 );
  fault_block_add_cell( block , 1 , 0 );
  fault_block_add_cell( block , 1 , 1 );
  fault_block_add_cell( block , 0 , 1 );


  test_assert_true( fault_block_layer_export( layer , ecl_kw1 ));
  test_assert_false( fault_block_layer_export( layer , ecl_kw2 ));
  test_assert_false( fault_block_layer_export( layer , ecl_kw3 ));

  {
    int nx = ecl_grid_get_nx( grid );
   
    test_assert_int_equal( ecl_kw_iget_int( ecl_kw1 , 0 ) , 10 );
    test_assert_int_equal( ecl_kw_iget_int( ecl_kw1 , 1 ) , 10 );
    test_assert_int_equal( ecl_kw_iget_int( ecl_kw1 , nx ) , 10 );
    test_assert_int_equal( ecl_kw_iget_int( ecl_kw1 , nx + 1 ) , 10 );
  }
  test_assert_int_equal( 40 , ecl_kw_element_sum_int( ecl_kw1 ));

  fault_block_layer_free( layer );
  ecl_kw_free( ecl_kw1 );
  ecl_kw_free( ecl_kw2 );
  ecl_kw_free( ecl_kw3 );
}


void test_neighbours( const ecl_grid_type * grid) {
  const int k = 0;
  fault_block_layer_type * layer = fault_block_layer_alloc( grid , k );
  geo_polygon_collection_type * polylines = geo_polygon_collection_alloc();
  ecl_kw_type * ecl_kw = ecl_kw_alloc("FAULTBLK" , ecl_grid_get_global_size( grid )     , ECL_INT);

  ecl_kw_iset_int( ecl_kw , 0 , 1);
  ecl_kw_iset_int( ecl_kw , ecl_grid_get_global_index3( grid , 3,3,k) , 2);
  ecl_kw_iset_int( ecl_kw , ecl_grid_get_global_index3( grid , 4,3,k) , 3);
  ecl_kw_iset_int( ecl_kw , ecl_grid_get_global_index3( grid , 5,3,k) , 4);
  ecl_kw_iset_int( ecl_kw , ecl_grid_get_global_index3( grid , 4,2,k) , 5);
  fault_block_layer_load_kw( layer , ecl_kw);
  
  {
    int_vector_type * neighbours = int_vector_alloc( 0,0);
    {
      fault_block_type * block = fault_block_layer_get_block( layer , 1 );

      test_assert_int_equal( 0 , int_vector_size( neighbours ));
      fault_block_list_neighbours( block , false , polylines , neighbours );
      test_assert_int_equal( 0 , int_vector_size( neighbours ));
    }

    {
      fault_block_type * block = fault_block_layer_get_block( layer , 2 );

      fault_block_list_neighbours( block , false , polylines , neighbours );
      test_assert_int_equal( 1 , int_vector_size( neighbours ));
      test_assert_true( int_vector_contains( neighbours , 3 ));
    }
    int_vector_free( neighbours );
    

  }
  
  geo_polygon_collection_free( polylines );
  fault_block_layer_free( layer );  
  ecl_kw_free( ecl_kw );
}
  


int main(int argc , char ** argv) {
  ecl_grid_type * ecl_grid = ecl_grid_alloc_rectangular( 9 , 9 , 2 , 1 , 1 , 1 , NULL );
  ecl_kw_type * fault_blk_kw = ecl_kw_alloc("FAULTBLK" , ecl_grid_get_global_size( ecl_grid ) , ECL_INT);

  test_create( ecl_grid , fault_blk_kw );
  test_create_invalid( ecl_grid );
  test_trace_edge( ecl_grid );
  test_export(ecl_grid);
  test_neighbours( ecl_grid );

  ecl_grid_free( ecl_grid );
  ecl_kw_free( fault_blk_kw );
  exit(0);
}


