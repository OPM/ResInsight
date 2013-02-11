/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'cv_enkf.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/rng.h>
#include <ert/util/matrix.h>

typedef struct cv_enkf_data_struct cv_enkf_data_type;

void * cv_enkf_data_alloc( rng_type * rng );
void   cv_enkf_data_free( void * arg );

void cv_enkf_init_update( void * arg , 
                          const matrix_type * S , 
                          const matrix_type * R , 
                          const matrix_type * dObs , 
                          const matrix_type * E , 
                          const matrix_type * D );

void cv_enkf_initX(void * module_data , 
                   matrix_type * X , 
                   matrix_type * A , 
                   matrix_type * S , 
                   matrix_type * R , 
                   matrix_type * dObs , 
                   matrix_type * E ,
                   matrix_type * D);

bool        cv_enkf_set_double( void * arg , const char * var_name , double value);
bool        cv_enkf_set_int( void * arg , const char * var_name , int value);
bool        cv_enkf_set_bool(  void * arg , const char * var_name , bool value );

void        cv_enkf_set_truncation( cv_enkf_data_type * data , double truncation );
void        cv_enkf_set_pen_press( cv_enkf_data_type * data , bool value );
void        cv_enkf_set_subspace_dimension( cv_enkf_data_type * data , int subspace_dimension);

