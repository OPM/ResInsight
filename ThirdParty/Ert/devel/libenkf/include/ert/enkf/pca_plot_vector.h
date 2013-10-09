/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'pca_plot_vector.h'
    
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
#ifndef __PCA_PLOT_VECTOR_H__
#define __PCA_PLOT_VECTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/util/matrix.h>

  typedef struct pca_plot_vector_struct pca_plot_vector_type;

  pca_plot_vector_type * pca_plot_vector_alloc( int component , const matrix_type * PC, const matrix_type * PC_obs);

  void                 pca_plot_vector_free( pca_plot_vector_type * plot_vector );
  bool                 pca_plot_assert_input( const matrix_type * PC, const matrix_type * PC_obs);
  void                 pca_plot_vector_free__( void * arg );
  double               pca_plot_vector_get_obs_value( const pca_plot_vector_type * vector );
  double               pca_plot_vector_iget_sim_value( const pca_plot_vector_type * vector , int sim_index);
  int                  pca_plot_vector_get_size( const pca_plot_vector_type * vector );

  UTIL_IS_INSTANCE_HEADER( pca_plot_vector );

#ifdef __cplusplus
} 
#endif
#endif
