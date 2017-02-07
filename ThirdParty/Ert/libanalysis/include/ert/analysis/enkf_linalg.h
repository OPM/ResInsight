#ifndef ERT_ENKF_LINALG_H
#define ERT_ENKF_LINALG_H

#include <ert/util/matrix_lapack.h>
#include <ert/util/matrix.h>
#include <ert/util/double_vector.h>


int enkf_linalg_get_PC( const matrix_type * S0, 
                         const matrix_type * dObs , 
                         double truncation,
                         int ncomp, 
                         matrix_type * PC,
                         matrix_type * PC_obs ,
                         double_vector_type * singular_values);

int enkf_linalg_num_PC(const matrix_type * S , double truncation );


void enkf_linalg_init_stdX( matrix_type * X , 
                            const matrix_type * S , 
                            const matrix_type * D , 
                            const matrix_type * W , 
                            const double * eig , 
                            bool bootstrap);


void enkf_linalg_init_sqrtX(matrix_type * X5      , 
                            const matrix_type * S , 
                            const matrix_type * randrot , 
                            const matrix_type * innov , 
                            const matrix_type * W , 
                            const double * eig , 
                            bool bootstrap);


void enkf_linalg_Cee(matrix_type * B, int nrens , const matrix_type * R , const matrix_type * U0 , const double * inv_sig0);


int enkf_linalg_svd_truncation(const matrix_type * S , 
                     double truncation , 
                     int ncomp ,
                     dgesvd_vector_enum store_V0T , 
                     double * sig0, 
                     matrix_type * U0 , 
                     matrix_type * V0T);


int enkf_linalg_svdS(const matrix_type * S , 
                     double truncation , 
                     int ncomp ,
                     dgesvd_vector_enum jobVT , 
                     double * sig0, 
                     matrix_type * U0 , 
                     matrix_type * V0T);



matrix_type * enkf_linalg_alloc_innov( const matrix_type * dObs , const matrix_type * S);

void enkf_linalg_lowrankCinv__(const matrix_type * S , 
                               const matrix_type * R , 
                               matrix_type * V0T , 
                               matrix_type * Z, 
                               double * eig , 
                               matrix_type * U0, 
                               double truncation, 
                               int ncomp);



void enkf_linalg_lowrankCinv(const matrix_type * S , 
                             const matrix_type * R , 
                             matrix_type * W       , /* Corresponding to X1 from Eq. 14.29 */
                             double * eig          , /* Corresponding to 1 / (1 + Lambda_1) (14.29) */
                             double truncation     ,
                             int    ncomp);

void enkf_linalg_lowrankE(const matrix_type * S , /* (nrobs x nrens) */
                          const matrix_type * E , /* (nrobs x nrens) */
                          matrix_type * W       , /* (nrobs x nrmin) Corresponding to X1 from Eqs. 14.54-14.55 */
                          double * eig          , /* (nrmin)         Corresponding to 1 / (1 + Lambda1^2) (14.54) */
                          double truncation     ,
                          int    ncomp);

void enkf_linalg_genX2(matrix_type * X2 , const matrix_type * S , const matrix_type * W , const double * eig);
void enkf_linalg_genX3(matrix_type * X3 , const matrix_type * W , const matrix_type * D , const double * eig);

void enkf_linalg_meanX5(const matrix_type * S , 
                        const matrix_type * W , 
                        const double * eig    , 
                        const matrix_type * innov ,
                        matrix_type * X5);


void enkf_linalg_X5sqrt(matrix_type * X2 , matrix_type * X5 , const matrix_type * randrot, int nrobs);

matrix_type * enkf_linalg_alloc_mp_randrot(int ens_size , rng_type * rng);
void          enkf_linalg_set_randrot( matrix_type * Q  , rng_type * rng);
void          enkf_linalg_checkX(const matrix_type * X , bool bootstrap);


//rml_enkf functions

void enkf_linalg_rml_enkfX1(matrix_type *X1, matrix_type * Udr ,matrix_type * S ,matrix_type *R);
void enkf_linalg_rml_enkfX2(matrix_type *X2, double  *Wdr, matrix_type * X1 ,double a , int nsign);
void enkf_linalg_rml_enkfX3(matrix_type *X3, matrix_type *VdTr, double *Wdr,matrix_type *X2, int nsign);

double enkf_linalg_data_mismatch(matrix_type *D , matrix_type *R , matrix_type *Sk);
void enkf_linalg_Covariance(matrix_type *Cd, const matrix_type *E, double nsc ,int nrobs);
void enkf_linalg_rml_enkfAm(matrix_type * Um, const double * Wm,int nsign1);

void enkf_linalg_rml_enkfX7(matrix_type * X7, matrix_type * VdT, double * Wdr, double a,matrix_type * X6);

#endif
