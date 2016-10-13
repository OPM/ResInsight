#ifndef ERT_RML_ENKF_COMMON_H
#define ERT_RML_ENKF_COMMON_H

#include <stdbool.h>

#include <ert/util/matrix.h>
#include <ert/util/rng.h>
#include <ert/util/bool_vector.h>


void rml_enkf_common_store_state( matrix_type * state , const matrix_type * A , const bool_vector_type * ens_mask );
void rml_enkf_common_recover_state( const matrix_type * state , matrix_type * A , const bool_vector_type * ens_mask );
void rml_enkf_common_scaleA(matrix_type *A , const double * Csc, bool invert );

#endif
