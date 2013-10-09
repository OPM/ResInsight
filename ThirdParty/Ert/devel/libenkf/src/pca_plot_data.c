/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'pca_plot_data.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <stdbool.h>

#include <ert/util/type_macros.h>
#include <ert/util/matrix.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>

#include <ert/enkf/pca_plot_data.h>
#include <ert/enkf/pca_plot_vector.h>

#define PCA_PLOT_DATA_TYPE_ID 61442098

struct pca_plot_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  char * name;
  vector_type * pca_vectors;
  int ens_size;
};


UTIL_IS_INSTANCE_FUNCTION( pca_plot_data , PCA_PLOT_DATA_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( pca_plot_data , PCA_PLOT_DATA_TYPE_ID )

static void pca_plot_data_add_vectors(pca_plot_data_type * plot_data , const matrix_type * PC , const matrix_type * PC_obs) {
  int component;
  for (component = 0; component < matrix_get_rows( PC ); component++) {
    pca_plot_vector_type * vector = pca_plot_vector_alloc( component , PC , PC_obs );
    vector_append_owned_ref( plot_data->pca_vectors , vector , pca_plot_vector_free__);
  }
}


pca_plot_data_type * pca_plot_data_alloc( const char * name, 
                                          const matrix_type * PC , 
                                          const matrix_type * PC_obs) {
  pca_plot_data_type * plot_data = NULL;

  if (pca_plot_assert_input( PC , PC_obs)) {    
    plot_data = util_malloc( sizeof * plot_data );
    UTIL_TYPE_ID_INIT( plot_data , PCA_PLOT_DATA_TYPE_ID );
    plot_data->name = util_alloc_string_copy( name );
    plot_data->pca_vectors = vector_alloc_new();
    plot_data->ens_size = matrix_get_columns( PC );
    pca_plot_data_add_vectors( plot_data , PC , PC_obs );
  }
  return plot_data;
}





void pca_plot_data_free( pca_plot_data_type * plot_data ) {
  vector_free( plot_data->pca_vectors );
  free( plot_data->name );
  free( plot_data );
}

void pca_plot_data_free__( void * arg ) {
  pca_plot_data_type * plot_data = pca_plot_data_safe_cast( arg );
  pca_plot_data_free( plot_data );
}

int pca_plot_data_get_size( const pca_plot_data_type * plot_data ) {
  return vector_get_size( plot_data->pca_vectors );
}


int pca_plot_data_get_ens_size( const pca_plot_data_type * plot_data ) {
  return plot_data->ens_size;
}

const pca_plot_vector_type * pca_plot_data_iget_vector( const pca_plot_data_type * plot_data , int ivec) {
  return vector_iget_const( plot_data->pca_vectors , ivec );
}


const char * pca_plot_data_get_name( const pca_plot_data_type * plot_data ) {
  return plot_data->name;
}
