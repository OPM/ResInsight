/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'rml_enkf.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>

#include <rml_enkf_common.h>


// Explanation
// zzz_enkf_common_store_state(  state , A ,ens_mask) assigns A to state. RESIZES state to rows(A)-by-LEN(ens_mask)
// zzz_enkf_common_recover_state(state , A ,ens_mask) assigns state to A. RESIZES A to rows(state)-by-SUM(ens_mask)


void rml_enkf_common_store_state( matrix_type * state , const matrix_type * A , const bool_vector_type * ens_mask ) {
  matrix_resize( state , matrix_get_rows( A ) , bool_vector_size( ens_mask ) , false);
  {
    const int ens_size = bool_vector_size( ens_mask );
    int active_index = 0;
    for (int iens = 0; iens < ens_size; iens++) {
      if (bool_vector_iget( ens_mask , iens )) {
        matrix_copy_column( state , A , iens , active_index );
        active_index++;
      } else
        matrix_set_const_column( state , iens  , 0);
    }
  }
}



void rml_enkf_common_recover_state( const matrix_type * state , matrix_type * A , const bool_vector_type * ens_mask ) {
  const int ens_size = bool_vector_size( ens_mask );
  const int active_size = bool_vector_count_equal( ens_mask , true );
  const int rows = matrix_get_rows( state );

  matrix_resize( A , rows , active_size , false );
  {
    int active_index = 0;
    for (int iens = 0; iens < ens_size; iens++) {
      if (bool_vector_iget( ens_mask , iens )) {
        matrix_copy_column( A , state , active_index , iens );
        active_index++;
      }
    }
  }
}



// Scale rows by the entries in the vector Csc
void rml_enkf_common_scaleA(matrix_type *A , const double * Csc, bool invert ){
  int nrows = matrix_get_rows(A);
  if (invert) {
    for (int i=0; i< nrows ; i++) {
      double sc= 1/Csc[i];
      matrix_scale_row(A, i, sc);
    }
  } else {
    for (int i=0; i< nrows ; i++) {
      double sc= Csc[i];
      matrix_scale_row(A, i, sc);
    }
  }
}
