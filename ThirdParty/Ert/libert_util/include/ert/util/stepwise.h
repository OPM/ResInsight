#ifndef ERT_STEPWISE_H
#define ERT_STEPWISE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/matrix.h>
#include <ert/util/bool_vector.h>

  typedef struct stepwise_struct stepwise_type;


  stepwise_type * stepwise_alloc1(int nsample, int nvar, rng_type * rng, const matrix_type* St, const matrix_type* Et);
  stepwise_type * stepwise_alloc0(rng_type * rng);
  void            stepwise_free( stepwise_type * stepwise);

  void            stepwise_set_Y0( stepwise_type * stepwise ,  matrix_type * Y);
  void            stepwise_set_X0( stepwise_type * stepwise ,  matrix_type * X);
  void            stepwise_set_E0( stepwise_type * stepwise ,  matrix_type * E);
  void            stepwise_set_beta( stepwise_type * stepwise ,  matrix_type * b);
  void            stepwise_set_R2( stepwise_type * stepwise ,  const double R2);
  void            stepwise_set_active_set( stepwise_type * stepwise ,  bool_vector_type * a);
  void            stepwise_isetY0( stepwise_type * stepwise , int i , double value );
  double          stepwise_get_R2(const stepwise_type * stepwise );
  int             stepwise_get_nvar( stepwise_type * stepwise );
  int             stepwise_get_nsample( stepwise_type * stepwise );
  int             stepwise_get_n_active( stepwise_type * stepwise );
  bool_vector_type * stepwise_get_active_set( stepwise_type * stepwise );
  double          stepwise_iget_beta(const stepwise_type * stepwise, const int index );
  double          stepwise_get_sum_beta(const stepwise_type * stepwise );

  void            stepwise_estimate( stepwise_type * stepwise , double deltaR2_limit , int CV_blocks);
  double          stepwise_eval( const stepwise_type * stepwise , const matrix_type * x );



#ifdef __cplusplus
}
#endif

#endif
