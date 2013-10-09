/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_pca_plot.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>
#include <ert/util/matrix.h>
#include <ert/util/rng.h>

#include <ert/enkf/pca_plot_data.h>
#include <ert/enkf/pca_plot_vector.h>



pca_plot_data_type * create_data() {
  matrix_type * PC = matrix_alloc( 3 , 10);
  matrix_type * PC_obs = matrix_alloc( 3 , 1 );
  pca_plot_data_type * data = pca_plot_data_alloc("KEY" , PC , PC_obs);
  
  matrix_free( PC );
  matrix_free( PC_obs );
  return data;
}



void test_create_data() {
  matrix_type * PC = matrix_alloc( 3 , 10);
  matrix_type * PC_obs = matrix_alloc( 3 , 1 );
  {
    pca_plot_data_type * data = pca_plot_data_alloc("KEY" , PC , PC_obs);
    test_assert_true( pca_plot_data_is_instance( data ));
    test_assert_int_equal( 3 , pca_plot_data_get_size( data ));
    test_assert_int_equal( 10 , pca_plot_data_get_ens_size( data ));
    test_assert_string_equal( "KEY" , pca_plot_data_get_name( data ));
    pca_plot_data_free( data );
  }
  matrix_resize( PC , 4 , 10 , false);
  test_assert_NULL( pca_plot_data_alloc( "KEY" , PC , PC_obs ));

  matrix_resize( PC_obs , 3 , 2 , false);
  test_assert_NULL( pca_plot_data_alloc( "KEY" , PC , PC_obs ));

  matrix_free( PC );
  matrix_free( PC_obs );
}



void test_create_vector() {
  matrix_type * PC = matrix_alloc( 3 , 10);
  matrix_type * PC_obs = matrix_alloc( 3 , 1 );
  {
    pca_plot_vector_type * vector = pca_plot_vector_alloc(0 , PC , PC_obs);
    test_assert_true( pca_plot_vector_is_instance( vector ));
    pca_plot_vector_free( vector );
  }
  matrix_free( PC );
  matrix_free( PC_obs );
}


void test_get_vector() {
  pca_plot_data_type * data = create_data();
  test_assert_true( pca_plot_vector_is_instance( pca_plot_data_iget_vector( data , 0 )));
  pca_plot_data_free( data );
}


void test_content() {
  rng_type * rng = rng_alloc(MZRAN , INIT_DEFAULT);
  matrix_type * PC = matrix_alloc( 3 , 10);
  matrix_type * PC_obs = matrix_alloc( 3 , 1 );
  matrix_random_init( PC , rng );
  matrix_random_init( PC_obs , rng );
  {
    pca_plot_data_type * data = pca_plot_data_alloc("KEY" , PC , PC_obs);
    for (int i=0; i < matrix_get_rows( PC ); i++) {
      const pca_plot_vector_type * vector = pca_plot_data_iget_vector( data , i );
      
      test_assert_double_equal( matrix_iget( PC_obs , i , 0) , 
                                pca_plot_vector_get_obs_value( vector ) );

      for (int j=0; j < matrix_get_columns( PC ); j++) 
        test_assert_double_equal( matrix_iget( PC , i , j ) , pca_plot_vector_iget_sim_value( vector , j ));
      
      test_assert_int_equal( matrix_get_columns( PC ) , pca_plot_vector_get_size( vector ));

    }
    pca_plot_data_free( data );
  }

  matrix_free( PC );
  matrix_free( PC_obs );
}


int main(int argc , char ** argv) {
  test_create_data();
  test_create_vector();
  test_get_vector();
  test_content();
  exit(0);
}

