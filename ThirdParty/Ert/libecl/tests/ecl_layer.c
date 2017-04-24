/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ecl_layer.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/struct_vector.h>

#include <ert/ecl/layer.h>


void test_create() {
  layer_type * layer = layer_alloc(10,20);
  test_assert_true( layer_is_instance( layer ));
  layer_free( layer );
}



void get_invalid_layer_cell1(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_cell_value( layer , 100 , 100 );
}


void get_invalid_layer_cell2(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_cell_value( layer , -100 , -100 );
}


void test_get_invalid_cell() {
  layer_type * layer = layer_alloc(10,10);
  test_assert_util_abort( "layer_get_global_cell_index" , get_invalid_layer_cell1 , layer );
  test_assert_util_abort( "layer_get_global_cell_index" , get_invalid_layer_cell2 , layer );
  layer_free( layer );
}


void test_get_cell() {
  layer_type * layer = layer_alloc(10,10);
  test_assert_int_equal( 0 , layer_iget_cell_value(layer , 0 ,0));
  test_assert_int_equal( 0 , layer_get_cell_sum( layer ));
  layer_iset_cell_value( layer , 0 , 0 , 77 );
  test_assert_int_equal( 77 , layer_get_cell_sum( layer ));
  test_assert_int_equal( 77 , layer_iget_cell_value(layer , 0 ,0));
  layer_iset_cell_value( layer , 1 , 1 , 23 );
  test_assert_int_equal( 100 , layer_get_cell_sum( layer ));
  layer_iset_cell_value( layer , 0 , 0 , 0 );
  test_assert_int_equal( 23 , layer_get_cell_sum( layer ));
  layer_free( layer );
}



void get_invalid_layer_edge1(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 10 , 10 , RIGHT_EDGE);
}


void get_invalid_layer_edge2(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 10 , 0, RIGHT_EDGE );
}


void get_invalid_layer_edge3(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 10 , 0, BOTTOM_EDGE );
}


void get_invalid_layer_edge4(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 0 , 10, TOP_EDGE );
}


void get_invalid_layer_edge5(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 0 , 10, RIGHT_EDGE );
}


void get_invalid_layer_edge6(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 0 , 10, LEFT_EDGE );
}


void get_invalid_layer_edge7(void * arg) {
  layer_type * layer = layer_safe_cast( arg );
  layer_iget_edge_value( layer , 10 , 0, TOP_EDGE );
}


void test_get_invalid_edge() {
  layer_type * layer = layer_alloc(10,10);
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge1 , layer );
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge2 , layer );
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge3 , layer );
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge4 , layer );
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge5 , layer );
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge6 , layer );
  test_assert_util_abort( "layer_get_global_edge_index" , get_invalid_layer_edge7 , layer );
  layer_free( layer );
}

void test_edge() {
  layer_type * layer = layer_alloc(10,10);

  layer_iset_cell_value(layer , 2,2,100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) , -100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) , -100);
  test_assert_true( layer_cell_on_edge( layer , 2 , 2 ));

  layer_iset_cell_value( layer , 3 , 2 , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) , -100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) , -100);

  test_assert_int_equal( layer_iget_edge_value(layer , 3 , 2 , LEFT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 3 , 2 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 3 , 2 , RIGHT_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 3 , 2 , TOP_EDGE) , -100);

  layer_iset_cell_value( layer , 1 , 2 , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) ,  0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) , -100);

  test_assert_int_equal( layer_iget_edge_value(layer , 1 , 2 , LEFT_EDGE) , -100);
  test_assert_int_equal( layer_iget_edge_value(layer , 1 , 2 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 1 , 2 , RIGHT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 1 , 2 , TOP_EDGE) , -100);

  layer_iset_cell_value( layer , 2 , 3 , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) ,  0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) ,   0);

  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 3 , LEFT_EDGE) , -100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 3 , BOTTOM_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 3 , RIGHT_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 3 , TOP_EDGE) , -100);


  layer_iset_cell_value( layer , 2 , 1 , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) ,  0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) ,0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) ,   0);

  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 1 , LEFT_EDGE) , -100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 1 , BOTTOM_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 1 , RIGHT_EDGE) , 100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 1 , TOP_EDGE) , 0);

  layer_iset_cell_value(layer , 2,2,100);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 0);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) , 0);
  test_assert_false( layer_cell_on_edge( layer , 2 , 2 ));


  layer_iset_cell_value(layer , 2,2,200);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , LEFT_EDGE) , -200);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , BOTTOM_EDGE) , 200);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , RIGHT_EDGE) , 200);
  test_assert_int_equal( layer_iget_edge_value(layer , 2 , 2 , TOP_EDGE) , -200);


  layer_free( layer );
}


void test_walk() {
  layer_type * layer = layer_alloc(10,10);
  struct_vector_type * corner_list = struct_vector_alloc( sizeof(int_point2d_type));
  int_vector_type * cell_list = int_vector_alloc(0,0);

  test_assert_false( layer_trace_block_edge( layer , 4 , 4 , 100 , corner_list , cell_list));
  layer_iset_cell_value( layer , 4,4,100 );
  test_assert_false( layer_trace_block_edge( layer , 4 , 4 , 200 , corner_list , cell_list));

  test_assert_true( layer_trace_block_edge( layer , 4 , 4 , 100 , corner_list , cell_list));
  test_assert_int_equal( struct_vector_get_size( corner_list ) , 4);
  test_assert_int_equal( int_vector_size( cell_list ) , 1 );
  {
    int_point2d_type point;

    struct_vector_iget( corner_list , 0 , &point);
    test_assert_int_equal( 4 , point.i );
    test_assert_int_equal( 4 , point.j );

    struct_vector_iget( corner_list , 1 , &point);
    test_assert_int_equal( 5 , point.i );
    test_assert_int_equal( 4 , point.j );

    struct_vector_iget( corner_list , 2 , &point);
    test_assert_int_equal( 5 , point.i );
    test_assert_int_equal( 5 , point.j );

    struct_vector_iget( corner_list , 3 , &point);
    test_assert_int_equal( 4 , point.i );
    test_assert_int_equal( 5 , point.j );
  }

  {
    int i,j;
    int_vector_type * true_cell_list = int_vector_alloc(0,0);
    for (j= 3; j < 7; j++) {
      for (i = 3; i < 7; i++) {
        layer_iset_cell_value( layer , i , j , 100 );

        if (i == 3 || j == 3)
          int_vector_append( true_cell_list , i + j*layer_get_nx( layer ));

        if (i == 6 || j == 6)
          int_vector_append( true_cell_list , i + j*layer_get_nx( layer ));

      }
    }
    int_vector_select_unique( true_cell_list );

    test_assert_true( layer_trace_block_edge( layer , 3 , 3 , 100 , corner_list , cell_list));
    test_assert_int_equal( 16 , struct_vector_get_size( corner_list ));
    test_assert_int_equal( 12 , int_vector_size( cell_list ));

    int_vector_fprintf( cell_list      , stdout , "     cell_list" , "%3d");
    int_vector_fprintf( true_cell_list , stdout , "true_cell_list" , "%3d");

    test_assert_true( int_vector_equal( cell_list , true_cell_list ));
    int_vector_free( true_cell_list );
  }

  int_vector_free( cell_list );
  struct_vector_free( corner_list );
  layer_free( layer );
}



void test_content1() {
  layer_type * layer = layer_alloc(10,10);
  int i,j;
  for (j=4; j < 8; j++)
    for (i=4; i < 8; i++)
      layer_iset_cell_value( layer , i , j , 1 );

  test_assert_int_equal( 16 , layer_get_cell_sum( layer ));
  {
    int_vector_type * i_list = int_vector_alloc(0,0);
    int_vector_type * j_list = int_vector_alloc(0,0);

    test_assert_false( layer_trace_block_content( layer , false , 4,4, 10 , i_list , j_list));
    test_assert_true( layer_trace_block_content( layer , false , 4,4, 1  , i_list , j_list ));
    test_assert_int_equal( 16 , int_vector_size( i_list ));

    int_vector_sort( i_list );
    int_vector_sort( j_list );

    for (j=0; j < 4; j++)
      for (i = 0; i < 4; i++) {
        test_assert_int_equal( int_vector_iget(i_list , i + j*4) , j + 4);
        test_assert_int_equal( int_vector_iget(j_list , i + j*4) , j + 4);
      }


    test_assert_true( layer_trace_block_content( layer , false , 4,4, 0  , i_list , j_list ));
    test_assert_int_equal( 16 , int_vector_size( i_list ));

    test_assert_true( layer_trace_block_content( layer ,true ,  4,4, 0  , i_list , j_list ));
    test_assert_int_equal( 16 , int_vector_size( i_list ));
    test_assert_int_equal( 0 , layer_get_cell_sum( layer ));

    int_vector_free( i_list );
    int_vector_free( j_list );
  }

  layer_free( layer );
}



void test_content2() {
  layer_type * layer = layer_alloc(5,5);
  int i,j;
  for (j=0; j < 5; j++)
    layer_iset_cell_value( layer , 2 , j , 1 );

  for (i=0; i < 5; i++)
    layer_iset_cell_value( layer , i , 2 , 1 );

  test_assert_int_equal( 9 , layer_get_cell_sum( layer ));
  {
    int_vector_type * i_list = int_vector_alloc(0,0);
    int_vector_type * j_list = int_vector_alloc(0,0);
    int_vector_type * cell_list = int_vector_alloc(0,0);
    struct_vector_type * corner_list = struct_vector_alloc( sizeof(int_point2d_type) );


    for (j=0; j < 5; j++) {
      for (i=0; i < 5; i++) {
        int cell_value = layer_iget_cell_value( layer , i , j );

        if (cell_value != 0) {
          test_assert_true( layer_trace_block_edge( layer , i , j , cell_value , corner_list , cell_list));
          test_assert_true( layer_trace_block_content( layer  , true , i,j,cell_value , i_list , j_list ));
        }

      }
    }

    struct_vector_free( corner_list );
    int_vector_free( i_list );
    int_vector_free( j_list );
    int_vector_free( cell_list );
  }
  test_assert_int_equal( 0 , layer_get_cell_sum( layer ));


  layer_free( layer );
}



void test_replace() {
  layer_type * layer = layer_alloc(10,10);
  int i,j;
  for (j = 0; j < 5; j++)
    for (i=0; i < 5; i++)
      layer_iset_cell_value( layer , i , j , 1 );

  test_assert_int_equal( 25 , layer_get_cell_sum( layer ));
  test_assert_int_equal( layer_replace_cell_values( layer , 1 , 2 ) , 25 );
  test_assert_int_equal( 50 , layer_get_cell_sum( layer ));
  test_assert_int_equal( layer_replace_cell_values( layer , 1 , 2 ) , 0 );

  layer_free( layer );
}



void test_interp_barrier() {
  layer_type * layer = layer_alloc(10,10);

  layer_add_interp_barrier( layer , 0 , 22 );

  layer_free( layer );
}



void test_copy( ) {
  layer_type * layer1 = layer_alloc(10,10);
  layer_type * layer2 = layer_alloc(10,10);

  layer_iset_cell_value( layer1 , 5,5,10 );
  layer_memcpy( layer2 , layer1 );

  test_assert_int_equal( 10 , layer_iget_edge_value( layer2 , 5,5,BOTTOM_EDGE));
  test_assert_int_equal( 10 , layer_iget_edge_value( layer2 , 5,5,RIGHT_EDGE));
  test_assert_int_equal( -10 , layer_iget_edge_value( layer2 , 5,5,TOP_EDGE));
  test_assert_int_equal( -10 , layer_iget_edge_value( layer2 , 5,5,LEFT_EDGE));

  layer_free( layer2 );
  layer_free( layer1 );
}


int main(int argc , char ** argv) {
  test_create();
  test_get_invalid_cell();
  test_get_cell();
  test_get_invalid_edge();
  test_edge();
  test_walk();
  test_content1();
  test_content2();
  test_replace();
  test_interp_barrier();
  test_copy();
}
