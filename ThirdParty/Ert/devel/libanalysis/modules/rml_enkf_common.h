
#ifndef __RML_ENKF_COMMON_H__
#define __RML_ENKF_COMMON_H__

#include <stdbool.h>

#include <ert/util/matrix.h>
#include <ert/util/rng.h>


void rml_enkf_common_initA__( matrix_type * A ,
                              matrix_type * S , 
                              matrix_type * Cd , 
                              matrix_type * E , 
                              matrix_type * D ,
                              double truncation,
                              double lamda,
                              matrix_type * Ud,
                              double * Wd,
                              matrix_type * VdT);


#endif
