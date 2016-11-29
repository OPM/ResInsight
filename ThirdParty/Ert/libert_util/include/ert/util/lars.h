/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lars.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef ERT_LARS_H
#define ERT_LARS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/matrix.h>

typedef struct lars_struct lars_type;

int               lars_get_sample( const lars_type * lars );
int               lars_get_nvar( const lars_type * lars );
lars_type       * lars_alloc1( int nsample , int nvars);
lars_type       * lars_alloc2( matrix_type * X , matrix_type * Y , bool internal_copy );
void              lars_estimate(lars_type * lars , int max_vars , double max_beta , bool verbose);
void              lars_isetX( lars_type * lars, int sample, int var , double value);
void              lars_isetY( lars_type * lars, int sample, double value);
void              lars_select_beta( lars_type * lars , int beta_index);
void              lars_free( lars_type * lars );
double            lars_eval1( const lars_type * lars , const matrix_type * x);
double            lars_eval2( const lars_type * lars , double * x);
double            lars_getY0( const lars_type * lars);
double            lars_iget_beta( const lars_type * lars , int index);

#ifdef __cplusplus
}
#endif
#endif
