/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'bootstrap_enkf.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/int_vector.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>

#include <ert/analysis/std_enkf.h>
#include <ert/analysis/cv_enkf.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/analysis_module.h>
#include <ert/analysis/enkf_linalg.h>

#define BOOTSTRAP_ENKF_TYPE_ID 741223

#define INVALID_SUBSPACE_DIMENSION  -1
#define INVALID_TRUNCATION          -1
#define DEFAULT_TRUNCATION          0.95
#define DEFAULT_NCOMP               INVALID_SUBSPACE_DIMENSION

#define  DEFAULT_DO_CV               false 
#define  DEFAULT_NFOLDS              10
#define  NFOLDS_KEY                  "BOOTSTRAP_NFOLDS"


typedef struct {
  UTIL_TYPE_ID_DECLARATION;
  std_enkf_data_type   * std_enkf_data;
  cv_enkf_data_type    * cv_enkf_data;
  rng_type             * rng; 
  long                   option_flags;
  bool                   doCV;  
} bootstrap_enkf_data_type;


static UTIL_SAFE_CAST_FUNCTION( bootstrap_enkf_data , BOOTSTRAP_ENKF_TYPE_ID )

/*****************************************************************/




//CV: static void lowrankCinv_pre_cv(const matrix_type * S , 
//CV:                                const matrix_type * R , 
//CV:                                matrix_type * V0T     , 
//CV:                                matrix_type * Z , 
//CV:                                double * eig , 
//CV:                                matrix_type * U0, 
//CV:                                double truncation , 
//CV:                                int ncomp) {
//CV:   
//CV:   const int nrobs = matrix_get_rows( S );
//CV:   const int nrens = matrix_get_columns( S );
//CV:   const int nrmin = util_int_min( nrobs , nrens );
//CV:   
//CV:   double * inv_sig0      = util_malloc( nrmin * sizeof * inv_sig0 , __func__);
//CV: 
//CV:   enkf_linalg_svdS( S , truncation , ncomp , DGESVD_MIN_RETURN , inv_sig0 , U0 , V0T);
//CV: 
//CV:   {  
//CV:     matrix_type * B    = matrix_alloc( nrmin , nrmin );
//CV:     enkf_linalg_Cee( B , nrens , R , U0 , inv_sig0);          /* B = Xo = (N-1) * Sigma0^(+) * U0'* Cee * U0 * Sigma0^(+')  (14.26)*/     
//CV:     /*USE SVD INSTEAD*/
//CV:     matrix_dgesvd(DGESVD_MIN_RETURN , DGESVD_NONE, B , eig, Z , NULL);
//CV:     matrix_free( B );
//CV:   }
//CV:   
//CV:   {
//CV:     int i,j;
//CV:     /* Lambda1 = (I + Lambda)^(-1) */
//CV:     
//CV:     for (i=0; i < nrmin; i++) 
//CV:       eig[i] = 1.0 / (1 + eig[i]);
//CV:     
//CV:     for (j=0; j < nrmin; j++)
//CV:       for (i=0; i < nrmin; i++)
//CV:         matrix_imul(Z , i , j , inv_sig0[i]); /* Z2 =  Sigma0^(+) * Z; */
//CV:   }
//CV: }                
//CV: 
//CV: 
//CV: 
//CV: void enkf_analysis_invertS_pre_cv(double truncation,
//CV:                                   int ncomp , 
//CV:                                   const matrix_type * S , 
//CV:                                   const matrix_type * R , 
//CV:                                   matrix_type * V0T , 
//CV:                                   matrix_type * Z , 
//CV:                                   double * eig , 
//CV:                                   matrix_type * U0 ) {
//CV:   
//CV:   lowrankCinv_pre_cv( S , R , V0T , Z , eig , U0 , truncation , ncomp );
//CV: }
//CV: 
//CV: 
//CV: 
//CV: static void getW_pre_cv(matrix_type * W , 
//CV:                         const matrix_type * V0T, 
//CV:                         const matrix_type * Z , 
//CV:                         double * eig , 
//CV:                         const matrix_type * U0 , 
//CV:                         int nfolds_CV, 
//CV:                         const matrix_type * A, 
//CV:                         int unique_bootstrap_components , 
//CV:                         rng_type * rng, 
//CV:                         bool pen_press) {
//CV: 
//CV:   const int nrobs = matrix_get_rows( U0 );
//CV:   const int nrens = matrix_get_columns( V0T );
//CV:   const int nrmin = util_int_min( nrobs , nrens );
//CV: 
//CV:   int i,j;
//CV:   
//CV:  
//CV:   /* Vector with random permutations of the itegers 1,...,nrens  */
//CV:   int * randperms     = util_malloc( sizeof * randperms * nrens, __func__);
//CV:   int * indexTest     = util_malloc( sizeof * indexTest * nrens, __func__);
//CV:   int * indexTrain    = util_malloc( sizeof * indexTrain * nrens, __func__);
//CV: 
//CV:   if(nrens != unique_bootstrap_components)
//CV:     nfolds_CV = util_int_min( nfolds_CV , unique_bootstrap_components-1);
//CV:   
//CV: 
//CV:   matrix_type * cvError = matrix_alloc( nrmin,nfolds_CV );
//CV:   
//CV:   /*Copy Z */
//CV:   matrix_type * workZ = matrix_alloc_copy( Z );
//CV: 
//CV:   int optP;
//CV:   
//CV:  
//CV:   /* start cross-validation: */
//CV: 
//CV:   const int maxp = matrix_get_rows(V0T);
//CV:   
//CV:   /* draw random permutations of the integers 0,...,nrens-1 */  
//CV:   for (i=0; i < nrens; i++)
//CV:     randperms[i] = i;
//CV:   rng_shuffle_int( rng , randperms , nrens );
//CV:   
//CV: 
//CV:   
//CV:   /*need to init cvError to all zeros (?) */
//CV:   for (i = 0; i < nrmin; i++){
//CV:     for( j = 0; j> nfolds_CV; j++){
//CV:       matrix_iset( cvError , i , j , 0.0 );
//CV:     }
//CV:   }
//CV:   
//CV:   int ntest, ntrain, k;
//CV:   printf("\nStarting cross-validation\n");
//CV:   for (i = 0; i < nfolds_CV; i++) {
//CV:     printf(".");
//CV: 
//CV:     ntest = 0;
//CV:     ntrain = 0;
//CV:     k = i;
//CV:     /*extract members for the training and test ensembles */
//CV:     for (j = 0; j < nrens; j++) {
//CV:       if (j == k) {
//CV:         indexTest[ntest] = randperms[j];
//CV:         k += nfolds_CV;
//CV:         ntest++;
//CV:       } else {
//CV:         indexTrain[ntrain] = randperms[j];
//CV:         ntrain++;
//CV:       }
//CV:     }
//CV:     
//CV:     //This one instaed: ?? enkf_analysis_get_cv_error_prin_comp( cv_data , cvError , A , indexTest , indexTrain, ntest, ntrain , i , maxP);
//CV:     enkf_analysis_get_cv_error( cvError , A , V0T , workZ , eig , indexTest , indexTrain, ntest, ntrain , i );
//CV:   }
//CV:   printf("\n");
//CV:   /* find optimal truncation value for the cv-scheme */
//CV:   optP = enkf_analysis_get_optimal_numb_comp( cvError , maxp, nfolds_CV, pen_press);
//CV: 
//CV:   printf("Optimal number of components found: %d \n",optP);
//CV:   printf("\n");
//CV:   FILE * compSel_log = util_fopen("compSel_log_local_cv" , "a");
//CV:   fprintf( compSel_log , " %d ",optP);
//CV:   fclose( compSel_log);
//CV: 
//CV: 
//CV:   /*free cvError vector and randperm */
//CV:   matrix_free( cvError );
//CV:   free( randperms );
//CV:   free( indexTest );
//CV:   free( indexTrain );
//CV: 
//CV:   /* need to update matrices so that we only use components 1,...,optP */
//CV:   /* remove non-zero entries of the z matrix (we do not want to recompute sigma0^(+') * z */
//CV:   /* this can surely be done much more efficiently, but for now we want to minimize the
//CV:      number of potential bugs in the code for now */ 
//CV:   for (i = optP; i < nrmin; i++) {
//CV:     for (j = 0; j < nrmin; j++) {
//CV:       matrix_iset(workZ , i , j, 0.0);
//CV:     }
//CV:   }
//CV:   
//CV:     
//CV:   /*fix the eig vector as well: */
//CV:   {
//CV:     int i;
//CV:     /* lambda1 = (i + lambda)^(-1) */
//CV:     for (i=optP; i < nrmin; i++) 
//CV:       eig[i] = 1.0;
//CV:   }
//CV: 
//CV:   matrix_matmul(W , U0 , workZ); /* x1 = w = u0 * z2 = u0 * sigma0^(+') * z    */
//CV:   
//CV: 
//CV: 
//CV: 
//CV:   /*end cross-validation */
//CV: }


// CV: void enkf_analysis_initX_pre_cv(matrix_type * X ,  
// CV:                                 int nfolds_CV,
// CV:                                 enkf_mode_type enkf_mode,
// CV:                                 bool bootstrap , 
// CV:                                 matrix_type * S , 
// CV:                                 matrix_type * R , 
// CV:                                 matrix_type * E , 
// CV:                                 matrix_type * D , 
// CV:                                 rng_type * rng , 
// CV:                                 meas_data_type * meas_data , 
// CV:                                 obs_data_type * obs_data , 
// CV:                                 const matrix_type * randrot , 
// CV:                                 const matrix_type * A , 
// CV:                                 const matrix_type * V0T , 
// CV:                                 const matrix_type * Z , 
// CV:                                 const double * eig , 
// CV:                                 const matrix_type * U0 , 
// CV:                                 meas_data_type * fasit , 
// CV:                                 int unique_bootstrap_components) {
// CV: 
// CV:   int ens_size               = meas_data_get_ens_size( meas_data );
// CV:   {
// CV:     int nrobs                = obs_data_get_active_size(obs_data);
// CV:     int nrmin                = util_int_min( ens_size , nrobs); 
// CV: 
// CV:     
// CV:     /*
// CV:       1: Allocating all matrices
// CV:     */
// CV:     /*Need a copy of A, because we need it later */
// CV:     matrix_type * workA      = matrix_alloc_copy( A );    /* <- This is a massive memory requirement. */
// CV:     matrix_type * innov      = enkf_linalg_alloc_innov( dObs , S );
// CV:     
// CV:     matrix_type * W          = matrix_alloc(nrobs , nrmin);                      
// CV:     double * workeig         = util_malloc( sizeof * workeig * nrmin , __func__);
// CV: 
// CV:     
// CV:     // Really unclear whether the row mean should have been shifted from S????
// CV:     
// CV:     /*copy entries in eig:*/
// CV:     {
// CV:       int i;
// CV:       for (i = 0 ; i < nrmin ; i++) 
// CV:         workeig[i] = eig[i];
// CV:     }
// CV:     
// CV:     /* Subtracting the ensemble mean of the state vector ensemble */
// CV:     matrix_subtract_row_mean( workA );
// CV:     
// CV:     /* 
// CV:        2: Diagonalize the S matrix; singular vectors are stored in W
// CV:           and singular values (after some massage) are stored in eig. 
// CV:           W = X1, eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
// CV:     */ 
// CV:     
// CV:     getW_pre_cv(W , V0T , Z , workeig ,  U0 , nfolds_CV , workA , unique_bootstrap_components , rng);
// CV:     
// CV:     /* 
// CV:        3: actually calculating the X matrix. 
// CV:     */
// CV:     switch (enkf_mode) {
// CV:     case(ENKF_STANDARD):
// CV:       enkf_linalg_init_stdX( X , S , D , W , workeig , bootstrap);
// CV:       break;
// CV:     case(ENKF_SQRT):
// CV:       enkf_linalg_init_sqrtX(X , S , randrot , dObs , W , workeig , bootstrap );
// CV:       break;
// CV:     default:
// CV:       util_abort("%s: INTERNAL ERROR \n",__func__);
// CV:     }
// CV:     
// CV:     matrix_free( W );
// CV:     matrix_free( R );
// CV:     matrix_free( S );
// CV:     matrix_free( workA );
// CV:     matrix_free( innov );
// CV:     free( workeig );
// CV:     
// CV:     if (enkf_mode == ENKF_STANDARD) {
// CV:       matrix_free( E );
// CV:       matrix_free( D );
// CV:     }
// CV:     
// CV:     enkf_analysis_checkX(X , bootstrap);
// CV:   }
// CV: }



             


/*****************************************************************/



void bootstrap_enkf_set_doCV( bootstrap_enkf_data_type * data , bool doCV) {
  data->doCV = doCV;
}



void bootstrap_enkf_set_truncation( bootstrap_enkf_data_type * boot_data , double truncation ) {
  std_enkf_set_truncation( boot_data->std_enkf_data , truncation );
  cv_enkf_set_truncation( boot_data->cv_enkf_data , truncation );
}


void bootstrap_enkf_set_subspace_dimension( bootstrap_enkf_data_type * boot_data , int ncomp) {
  std_enkf_set_subspace_dimension( boot_data->std_enkf_data , ncomp );
  cv_enkf_set_subspace_dimension( boot_data->cv_enkf_data , ncomp );
}


void * bootstrap_enkf_data_alloc( rng_type * rng ) {
  bootstrap_enkf_data_type * boot_data = util_malloc( sizeof * boot_data );
  UTIL_TYPE_ID_INIT( boot_data , BOOTSTRAP_ENKF_TYPE_ID );

  boot_data->std_enkf_data = std_enkf_data_alloc( NULL );
  boot_data->cv_enkf_data = cv_enkf_data_alloc( rng );
  
  boot_data->rng = rng;
  bootstrap_enkf_set_truncation( boot_data , DEFAULT_TRUNCATION );
  bootstrap_enkf_set_subspace_dimension( boot_data , DEFAULT_NCOMP );
  bootstrap_enkf_set_doCV( boot_data , DEFAULT_DO_CV);
  boot_data->option_flags = ANALYSIS_NEED_ED + ANALYSIS_UPDATE_A + ANALYSIS_SCALE_DATA;
  return boot_data;
}





void bootstrap_enkf_data_free( void * arg ) {
  bootstrap_enkf_data_type * boot_data = bootstrap_enkf_data_safe_cast( arg );
  {
    std_enkf_data_free( boot_data->std_enkf_data );
    cv_enkf_data_free( boot_data->cv_enkf_data );
  }
}


static int ** alloc_iens_resample( rng_type * rng , int ens_size ) {
  int ** iens_resample;
  int iens;

  iens_resample = util_calloc( ens_size , sizeof * iens_resample );
  for (iens = 0; iens < ens_size; iens++)
    iens_resample[iens] = util_calloc( ens_size , sizeof( ** iens_resample ) );
  
  {
    int i,j;
    for (i=0; i < ens_size; i++)
      for (j=0; j < ens_size; j++) 
        iens_resample[i][j] = rng_get_int( rng , ens_size );
  }
  return iens_resample;
}


static void free_iens_resample( int ** iens_resample, int ens_size ) {
  for (int i=0; i < ens_size; i++)
    free( iens_resample[i] );
  free( iens_resample );
}



void bootstrap_enkf_updateA(void * module_data , 
                            matrix_type * A , 
                            matrix_type * S , 
                            matrix_type * R , 
                            matrix_type * dObs , 
                            matrix_type * E ,
                            matrix_type * D , 
                            matrix_type * randrot) {
  
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( module_data );
  {
    const int num_cpu_threads = 4;
    int ens_size              = matrix_get_columns( A );
    matrix_type * X           = matrix_alloc( ens_size , ens_size );
    matrix_type * A0          = matrix_alloc_copy( A );
    matrix_type * S_resampled = matrix_alloc_copy( S );
    matrix_type * A_resampled = matrix_alloc( matrix_get_rows(A0) , matrix_get_columns( A0 ));
    int ** iens_resample      = alloc_iens_resample( bootstrap_data->rng , ens_size );
    {
      int ensemble_members_loop;
      for ( ensemble_members_loop = 0; ensemble_members_loop < ens_size; ensemble_members_loop++) { 
        int unique_bootstrap_components;
        int ensemble_counter;
        /* Resample A and meas_data. Here we are careful to resample the working copy.*/
        {
          {
            int_vector_type * bootstrap_components = int_vector_alloc( ens_size , 0);
            for (ensemble_counter  = 0; ensemble_counter < ens_size; ensemble_counter++) {
              int random_column = iens_resample[ ensemble_members_loop][ensemble_counter];
              int_vector_iset( bootstrap_components , ensemble_counter , random_column );
              matrix_copy_column( A_resampled , A0 , ensemble_counter , random_column );
              matrix_copy_column( S_resampled , S  , ensemble_counter , random_column );
            }
            int_vector_select_unique( bootstrap_components );
            unique_bootstrap_components = int_vector_size( bootstrap_components );
            int_vector_free( bootstrap_components );
          }
          
          if (bootstrap_data->doCV) {
            cv_enkf_init_update( bootstrap_data->cv_enkf_data , S_resampled , R , dObs , E , D);
            cv_enkf_initX( bootstrap_data->cv_enkf_data , X , A_resampled , S_resampled , R , dObs , E , D);
          } else 
            std_enkf_initX(bootstrap_data->std_enkf_data , X , NULL , S_resampled,R, dObs, E,D );


          matrix_inplace_matmul_mt1( A_resampled , X , num_cpu_threads );
          matrix_inplace_add( A_resampled , A0 );
          matrix_copy_column( A , A_resampled, ensemble_members_loop, ensemble_members_loop);

        }
      }
    }
    

    free_iens_resample( iens_resample , ens_size);
    matrix_free( X );
    matrix_free( S_resampled );
    matrix_free( A_resampled );
    matrix_free( A0 );
  }
}






long bootstrap_enkf_get_options( void * arg , long flag) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    return bootstrap_data->option_flags;
  }
}


bool bootstrap_enkf_set_double( void * arg , const char * var_name , double value) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    if (std_enkf_set_double( bootstrap_data->std_enkf_data , var_name , value ))
      return true;
    else {
      return false;
    }
  }
}


bool bootstrap_enkf_set_int( void * arg , const char * var_name , int value) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    if (std_enkf_set_int( bootstrap_data->std_enkf_data , var_name , value ))
      return true;
    else {
      return false;
    }
  }
}


bool bootstrap_enkf_set_bool( void * arg , const char * var_name , bool value) {
  bootstrap_enkf_data_type * bootstrap_data = bootstrap_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , "CV" ) == 0)
      bootstrap_data->doCV = value;
    else
      name_recognized = false;

    return name_recognized;
  }
}






#ifdef INTERNAL_LINK
#define SYMBOL_TABLE bootstrap_enkf_symbol_table
#else
#define SYMBOL_TABLE EXTERNAL_MODULE_SYMBOL
#endif




analysis_table_type SYMBOL_TABLE = {
  .alloc           = bootstrap_enkf_data_alloc,
  .freef           = bootstrap_enkf_data_free,
  .set_int         = bootstrap_enkf_set_int , 
  .set_double      = bootstrap_enkf_set_double , 
  .set_bool        = bootstrap_enkf_set_bool , 
  .set_string      = NULL , 
  .get_options     = bootstrap_enkf_get_options , 
  .initX           = NULL,
  .updateA         = bootstrap_enkf_updateA, 
  .init_update     = NULL,
  .complete_update = NULL, 
  .has_var         = NULL,
  .get_int         = NULL,
  .get_double      = NULL,
  .get_ptr         = NULL, 
};
