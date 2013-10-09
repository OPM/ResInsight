/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'pca_plot_vector.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/enkf/pca_plot_vector.h>

#define PCA_PLOT_VECTOR_TYPE_ID 61743098

struct pca_plot_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int      size;
  double   obs_value;
  double * sim_data;
};


UTIL_IS_INSTANCE_FUNCTION( pca_plot_vector , PCA_PLOT_VECTOR_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( pca_plot_vector , PCA_PLOT_VECTOR_TYPE_ID )

bool pca_plot_assert_input( const matrix_type * PC, const matrix_type * PC_obs) {
  if ((matrix_get_rows(PC) == matrix_get_rows( PC_obs )) && 
      (matrix_get_columns(PC_obs) == 1)) 
    return true;
  else
    return false;
}

static void pca_plot_vector_init_data( pca_plot_vector_type * plot_vector , int component, const matrix_type * PC , const matrix_type * PC_obs) {
  int iens;
  
  for (iens = 0; iens < matrix_get_columns( PC ); iens++)
    plot_vector->sim_data[iens] = matrix_iget( PC, component , iens );

  plot_vector->obs_value = matrix_iget( PC_obs , component , 0 );
}

pca_plot_vector_type * pca_plot_vector_alloc( int component , 
                                              const matrix_type * PC , 
                                              const matrix_type * PC_obs) {
  pca_plot_vector_type * plot_vector = NULL;

  if (pca_plot_assert_input( PC , PC_obs) && (component < matrix_get_rows( PC ))) {
    
    plot_vector = util_malloc( sizeof * plot_vector );
    UTIL_TYPE_ID_INIT( plot_vector , PCA_PLOT_VECTOR_TYPE_ID );
    plot_vector->obs_value = matrix_iget( PC_obs , component , 0 );
    plot_vector->size = matrix_get_columns( PC );
    plot_vector->sim_data = util_calloc( plot_vector->size , sizeof * plot_vector->sim_data );
    pca_plot_vector_init_data( plot_vector , component , PC , PC_obs );
  }

  return plot_vector;
}



void pca_plot_vector_free( pca_plot_vector_type * plot_vector ) {
  free( plot_vector->sim_data );
  free( plot_vector );
}


void pca_plot_vector_free__( void * arg ) {
  pca_plot_vector_type * vector = pca_plot_vector_safe_cast( arg );
  pca_plot_vector_free( vector );
}


int pca_plot_vector_get_size( const pca_plot_vector_type * vector ) {
  return vector->size;
}

double pca_plot_vector_get_obs_value( const pca_plot_vector_type * vector ) {
  return vector->obs_value;
}


double pca_plot_vector_iget_sim_value( const pca_plot_vector_type * vector , int sim_index) {
  return vector->sim_data[ sim_index ];
}
