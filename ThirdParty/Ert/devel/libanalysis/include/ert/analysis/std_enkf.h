#ifndef __STD_ENKF_H__
#define __STD_ENKF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/matrix.h>
#include <ert/util/rng.h>

#define  DEFAULT_ENKF_TRUNCATION_  0.98
#define  ENKF_TRUNCATION_KEY_      "ENKF_TRUNCATION"
#define  ENKF_NCOMP_KEY_           "ENKF_NCOMP" 

  typedef struct std_enkf_data_struct std_enkf_data_type;


  void     std_enkf_initX(void * module_data , matrix_type * X , matrix_type * A , matrix_type * S , matrix_type * R , matrix_type * innov , matrix_type * E , matrix_type *D );
  bool     std_enkf_set_double( void * arg , const char * var_name , double value);
  
  bool     std_enkf_set_int( void * arg , const char * var_name , int value);
  int      std_enkf_get_subspace_dimension( std_enkf_data_type * data );
  void     std_enkf_set_truncation( std_enkf_data_type * data , double truncation );
  void     std_enkf_set_subspace_dimension( std_enkf_data_type * data , int subspace_dimension);
  
  
  double   std_enkf_get_truncation( std_enkf_data_type * data );
  void   * std_enkf_data_alloc( rng_type * rng);
  void     std_enkf_data_free( void * module_data );
  
  void     std_enkf_initX__( matrix_type * X , 
                             matrix_type * S , 
                             matrix_type * R , 
                             matrix_type * E , 
                             matrix_type * D ,
                             double truncation,
                             int    ncomp,
                             bool   bootstrap );
  
  
  
#ifdef __cplusplus
}
#endif

#endif





