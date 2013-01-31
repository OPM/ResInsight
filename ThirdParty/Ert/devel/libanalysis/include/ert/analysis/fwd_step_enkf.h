/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'fwd_step_enkf.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

typedef struct fwd_step_enkf_data_struct fwd_step_enkf_data_type;

void * fwd_step_enkf_data_alloc( rng_type * rng );
void   fwd_step_enkf_data_free( void * arg );

void fwd_step_enkf_updateA(void * module_data , 
                            matrix_type * A , 
                            matrix_type * S , 
                            matrix_type * R , 
                            matrix_type * dObs , 
                            matrix_type * E ,
                            matrix_type * D );


void        fwd_step_enkf_set_truncation( fwd_step_enkf_data_type * data , double truncation );
void        fwd_step_enkf_set_subspace_dimension( fwd_step_enkf_data_type * data , int subspace_dimension);

