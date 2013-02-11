/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'null_enkf.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>


void null_enkf_initX(void * module_data , 
                    matrix_type * X , 
                    matrix_type * A , 
                    matrix_type * S , 
                    matrix_type * R , 
                    matrix_type * dObs , 
                    matrix_type * E , 
                    matrix_type * D) {

  matrix_diag_set_scalar( X , 1.0 );

}


long null_enkf_get_options( void * arg , long flag ) {
  return 0L;
}



/**
   gcc -fpic -c <object_file> -I??  <src_file>
   gcc -shared -o <lib_file> <object_files>
*/



#ifdef INTERNAL_LINK
#define SYMBOL_TABLE null_enkf_symbol_table
#else
#define SYMBOL_TABLE EXTERNAL_MODULE_SYMBOL
#endif

analysis_table_type SYMBOL_TABLE = {
    .alloc           = NULL ,
    .freef           = NULL ,
    .set_int         = NULL ,
    .set_double      = NULL ,
    .set_bool        = NULL , 
    .set_string      = NULL , 
    .get_options     = null_enkf_get_options,
    .initX           = null_enkf_initX , 
    .updateA         = NULL,
    .init_update     = NULL,
    .complete_update = NULL,
    .has_var         = NULL,
    .get_int         = NULL,
    .get_double      = NULL,
    .get_ptr         = NULL, 
};

