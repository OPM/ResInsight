/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'pca_plot_data.h'
    
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
#ifndef __PCA_PLOT_DATA_H__
#define __PCA_PLOT_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/util/matrix.h>

#include <ert/enkf/pca_plot_vector.h>

  typedef struct pca_plot_data_struct pca_plot_data_type;

  pca_plot_data_type * pca_plot_data_alloc( const char * name , const matrix_type * PC, const matrix_type * PC_obs);

  void                 pca_plot_data_free( pca_plot_data_type * plot_data );
  const pca_plot_vector_type * pca_plot_data_iget_vector( const pca_plot_data_type * plot_data , int ivec);
  int                  pca_plot_data_get_size( const pca_plot_data_type * plot_data );
  const char         * pca_plot_data_get_name( const pca_plot_data_type * plot_data );
  int                  pca_plot_data_get_ens_size( const pca_plot_data_type * plot_data );
  void                 pca_plot_data_free__( void * arg );

  UTIL_IS_INSTANCE_HEADER( pca_plot_data );

#ifdef __cplusplus
} 
#endif
#endif
