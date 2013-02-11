#ifndef __STEPWISE_H__
#define __STEPWISE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/matrix.h>
#include <ert/util/bool_vector.h>

  typedef struct stepwise_struct stepwise_type;


  stepwise_type * stepwise_alloc2( matrix_type * X , matrix_type * Y , bool internal_copy , rng_type * rng);
  stepwise_type * stepwise_alloc1(int nsample, int nvar, rng_type * rng);
  stepwise_type * stepwise_alloc0(rng_type * rng);

  void            stepwise_free( stepwise_type * stepwise);
  void            stepwise_estimate( stepwise_type * stepwise , double deltaR2_limit , int CV_blocks);
  double          stepwise_eval( const stepwise_type * stepwise , const matrix_type * x );
  void            stepwise_set_Y0( stepwise_type * stepwise ,  matrix_type * Y);
  void            stepwise_set_X0( stepwise_type * stepwise ,  matrix_type * X);
  void            stepwise_set_beta( stepwise_type * stepwise ,  matrix_type * b);
  void            stepwise_set_active_set( stepwise_type * stepwise ,  bool_vector_type * a);
  void            stepwise_isetY0( stepwise_type * stepwise , int i , double value );

  int stepwise_get_nvar( stepwise_type * stepwise );
  int stepwise_get_nsample( stepwise_type * stepwise );
  
  
#ifdef __cplusplus
}
#endif

#endif
