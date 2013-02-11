/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'cv_enkf.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/rng.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>

#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/analysis_module.h>
#include <ert/analysis/std_enkf.h>
#include <ert/analysis/cv_enkf.h>


#define CV_ENKF_TYPE_ID 765523

#define INVALID_SUBSPACE_DIMENSION  -1
#define INVALID_TRUNCATION          -1
#define DEFAULT_SUBSPACE_DIMENSION  INVALID_SUBSPACE_DIMENSION

#define DEFAULT_NFOLDS              10
#define DEFAULT_PEN_PRESS           false
#define NFOLDS_KEY                  "CV_NFOLDS"
#define CV_PEN_PRESS_KEY            "CV_PEN_PRESS"




struct cv_enkf_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  matrix_type          * Z;
  matrix_type          * Rp;
  matrix_type          * Dp;
  rng_type             * rng;
  double                 truncation;
  int                    nfolds;
  int                    subspace_dimension;  // ENKF_NCOMP_KEY (-1: use Truncation instead)
  long                   option_flags;
  bool                   penalised_press;
};



static UTIL_SAFE_CAST_FUNCTION( cv_enkf_data , CV_ENKF_TYPE_ID )


void cv_enkf_set_truncation( cv_enkf_data_type * data , double truncation ) {
  data->truncation = truncation;
  if (truncation > 0.0)
    data->subspace_dimension = INVALID_SUBSPACE_DIMENSION;
}


void cv_enkf_set_subspace_dimension( cv_enkf_data_type * data , int subspace_dimension) {
  data->subspace_dimension = subspace_dimension;
  if (subspace_dimension > 0)
    data->truncation = INVALID_TRUNCATION;
}


void cv_enkf_set_nfolds( cv_enkf_data_type * data , int nfolds ) {
  data->nfolds = nfolds;
}

void cv_enkf_set_pen_press( cv_enkf_data_type * data , bool value ) {
  data->penalised_press = value;
}


void * cv_enkf_data_alloc( rng_type * rng ) {
  cv_enkf_data_type * data = util_malloc( sizeof * data);
  UTIL_TYPE_ID_INIT( data , CV_ENKF_TYPE_ID );

  data->Z            = NULL;
  data->Rp           = NULL;
  data->Dp           = NULL;
  data->rng          = rng;

  data->penalised_press = DEFAULT_PEN_PRESS;
  data->option_flags    = ANALYSIS_NEED_ED + ANALYSIS_USE_A + ANALYSIS_SCALE_DATA;
  data->nfolds          = DEFAULT_NFOLDS;
  cv_enkf_set_truncation( data , DEFAULT_ENKF_TRUNCATION_ );
  
  return data;
}



void cv_enkf_data_free( void * arg ) {
  cv_enkf_data_type * cv_data = cv_enkf_data_safe_cast( arg );
  {
    matrix_safe_free( cv_data->Z );
    matrix_safe_free( cv_data->Rp );
    matrix_safe_free( cv_data->Dp );
  }
}






void cv_enkf_init_update( void * arg , 
                          const matrix_type * S , 
                          const matrix_type * R , 
                          const matrix_type * dObs , 
                          const matrix_type * E , 
                          const matrix_type * D ) {

  cv_enkf_data_type * cv_data = cv_enkf_data_safe_cast( arg );
  {
    int i, j;
    const int nrobs = matrix_get_rows( S );
    const int nrens = matrix_get_columns( S );
    const int nrmin = util_int_min( nrobs , nrens );

    cv_data->Z   = matrix_alloc( nrmin , nrens );
    cv_data->Rp  = matrix_alloc( nrmin , nrmin );
    cv_data->Dp  = matrix_alloc( nrmin , nrens );
    
    /*
      Compute SVD(S)
    */
    matrix_type * U0   = matrix_alloc( nrobs , nrmin    ); /* Left singular vectors.  */
    matrix_type * V0T  = matrix_alloc( nrmin , nrens );    /* Right singular vectors. */
    
    double * inv_sig0  = util_calloc( nrmin , sizeof * inv_sig0 );
    double * sig0      = inv_sig0;
    
    printf("Computing svd using truncation %0.4f\n",cv_data->truncation);

    enkf_linalg_svdS(S , cv_data->truncation , cv_data->subspace_dimension , DGESVD_MIN_RETURN , inv_sig0 , U0 , V0T);
    
    /* Need to use the original non-inverted singular values. */
    for(i = 0; i < nrmin; i++) 
      if ( inv_sig0[i] > 0 ) 
        sig0[i] = 1.0 / inv_sig0[i];
    
   /*
      Compute the actual principal components, Z = sig0 * VOT 
      NOTE: Z contains potentially alot of redundant zeros, but 
      we do not care about this for now
    */
    
    for(i = 0; i < nrmin; i++) 
      for(j = 0; j < nrens; j++) 
        matrix_iset( cv_data->Z , i , j , sig0[i] * matrix_iget( V0T , i , j ) );
    
    /* Also compute Rp */
    {
      matrix_type * X0 = matrix_alloc( nrmin , matrix_get_rows( R ));
      matrix_dgemm(X0 , U0 , R  , true  , false , 1.0 , 0.0);   /* X0 = U0^T * R */
      matrix_dgemm(cv_data->Rp  , X0 , U0 , false , false , 1.0 , 0.0);  /* Rp = X0 * U0 */
      matrix_free(X0);
    }

    /*We also need to compute the reduced "Innovation matrix" Dp = U0' * D    */
    matrix_dgemm(cv_data->Dp , U0 , D , true , false , 1.0 , 0.0);
    
    
    free(inv_sig0);
    matrix_free(U0);
    matrix_free(V0T);
    
    /* 
       2: Diagonalize the S matrix; singular vectors etc. needed later in the local CV:
       (V0T = transposed right singular vectors of S, Z = scaled principal components, 
       eig = scaled, inverted singular vectors, U0 = left singular vectors of S
       eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
    */ 
  }
}



/*Function that computes the PRESS for different subspace dimensions using
  m-fold CV 
  INPUT :
  A   : State-Vector ensemble matrix
  Z   : Ensemble matrix of principal components
  Rp  : Reduced order Observation error matrix
  indexTrain: index of training ensemble
  indexTest: index of test ensemble
  nTest : number of members in the training ensemble
  nTrain . number of members in the test ensemble
  foldIndex: integer specifying which "cv-fold" we are considering
  
  OUTPUT:
  cvErr : UPDATED MATRIX OF PRESS VALUES
  
*/

static void cv_enkf_get_cv_error_prin_comp( cv_enkf_data_type * cv_data , 
                                                  matrix_type * cvErr , 
                                                  const matrix_type * A ,  
                                                  const int * indexTest, 
                                                  const int * indexTrain , 
                                                  const int nTest , 
                                                  const int nTrain , 
                                                  const int foldIndex, 
                                                  const int maxP) { 
  /*  
      We need to predict ATest(p), for p = 1,...,nens -1, based on the estimated regression model:
      AHatTest(p) = A[:,indexTrain] * Z[1:p,indexTrain]'* inv( Z[1:p,indexTrain] * Z[1:p,indexTrain]' + (nens-1) * Rp[1:p,1:p] ) * Z[1:p,indexTest];
  */
  

  const int nx         = matrix_get_rows( A );
  matrix_type * AHat   = matrix_alloc(nx , nTest );
  matrix_type * ATrain = matrix_alloc( nx , nTrain );
  
  int p,i,j;
    
  
  /* Copy elements*/
  for (i = 0; i < nx; i++) 
    for (j = 0; j < nTrain; j++) 
      matrix_iset(ATrain , i , j , matrix_iget( A , i , indexTrain[j]));
  
  
  for (p = 0; p < maxP; p++) {
    matrix_type * ZpTrain = matrix_alloc( p + 1, nTrain );
    matrix_type *SigDp    = matrix_alloc( p + 1 , p + 1);
    
    
    for (i = 0; i <= p ; i++) 
      for (j = 0; j < nTrain; j++) 
        matrix_iset(ZpTrain , i , j , matrix_iget(cv_data->Z , i , indexTrain[j]));
    
    /* SigDp = ZpTrain * ZpTrain' */
    matrix_dgemm( SigDp , ZpTrain , ZpTrain, false , true , 1.0, 0.0);

    /* SigDp += (nTrain - 1) * Rp */
    for(i = 0; i <= p; i++) 
      for( j = 0; j <= p; j++) 
        matrix_iadd( SigDp , i , j , (nTrain - 1) * matrix_iget(cv_data->Rp, i, j));
    
    
    /* Invert the covariance matrix for the principal components  */
    {
      int inv_ok = matrix_inv( SigDp );
      if ( inv_ok != 0 ) 
        util_abort("%s: inversion of covariance matrix for the principal components failed for subspace dimension p = %d\n - aborting \n",__func__,p+1); 
    }

    
    {
      matrix_type * W  = matrix_alloc(p + 1 , nTest );
      matrix_type * W2 = matrix_alloc(nTrain , nTest );

      /* W = inv(SigDp) * ZTest */
      for (i = 0; i <= p; i++) {
        for (j = 0; j < nTest; j++) {
          double ksum = 0.0;
          for (int k = 0; k <= p; k++) 
            ksum += matrix_iget(SigDp , i , k) * matrix_iget(cv_data->Z , k , indexTest[j]);
          
          matrix_iset(W , i , j , ksum);
        }
      }

      /* W2 = ZpTrain' * W */
      matrix_dgemm( W2 , ZpTrain , W , true , false , 1.0 , 0.0);
      
      /* Estimate the state-vector */
      matrix_matmul( AHat , ATrain , W2 );

      matrix_free( W2 );
      matrix_free( W );
    }

    
    /*Compute Press Statistic: */
    {
      double R2Sum = 0;

      for (i = 0; i < nx; i++) {
        for (j = 0; j < nTest; j++) {
          double tmp = matrix_iget(A , i , indexTest[j]) - matrix_iget(AHat , i , j);
          R2Sum += tmp * tmp;
        }
      }
      matrix_iset( cvErr , p , foldIndex , R2Sum );
    }
    
    
    
    matrix_free( ZpTrain );
    matrix_free( SigDp );
  } /*end for p */
  
  matrix_free( AHat );
  matrix_free( ATrain );
}






int cv_enkf_get_optimal_numb_comp(cv_enkf_data_type * cv_data , 
                                        const matrix_type * cvErr , 
                                        const int maxP ) {
                                        

  double * cvMean = util_calloc( maxP , sizeof * cvMean );
  double * cvStd  = util_calloc( maxP , sizeof * cvStd  );
  int optP;

  {
    for (int p = 0; p < maxP; p++ ){
      double err_sum = 0;
      for (int folds = 0; folds < cv_data->nfolds; folds++ )
        err_sum += matrix_iget( cvErr , p, folds );
      
      cvMean[p] = err_sum / cv_data->nfolds;
    }
    
    for ( int p = 0; p < maxP; p++){
      double err_sum2 = 0;
      for ( int folds = 0; folds < cv_data->nfolds; folds++)
        err_sum2 += pow( matrix_iget( cvErr , p , folds ) - cvMean[p] , 2);
      
      cvStd[p] = sqrt( err_sum2 / (cv_data->nfolds - 1) );
    }
  }
  
  {
    double minErr = cvMean[0];
    int i;
    optP = 1;
    
    printf("PRESS:\n");
    printf("%f\n",cvMean[0]);
    for (i = 1; i < maxP; i++) {
      printf("%f\n",cvMean[i]);
      if ((cvMean[i] < minErr) && (cvMean[i] > 0.0)) {
        minErr = cvMean[i];
        optP = i+1;
      }
    }



  
    if (cv_data->penalised_press) {
      for ( i = 0; i < optP; i++){
        if( cvMean[i] - cvStd[i] <= minErr ){
          optP = i+1;
          break;
        }
      }
    }
  }    
  
  free( cvStd );
  free( cvMean );
  return optP;
}






/* Function that performs cross-validation to find the optimal subspace dimension,  */


static int get_optimal_principal_components( cv_enkf_data_type * cv_data , 
                                             const matrix_type * A) {
  

  const int nrens = matrix_get_columns( cv_data->Z );
  const int nrmin = matrix_get_rows( cv_data->Z );



  matrix_type * cvError;
  int * randperms     = util_calloc( nrens , sizeof * randperms);
  
  int maxP  = nrmin;
  int optP;
      
  
  /* We only want to search the non-zero eigenvalues */
  for (int i = 0; i < nrmin; i++) {
    if (matrix_iget(cv_data->Z,i,1) == 0.0) {
      maxP = i;
      break;
    }
  }
  
  if (maxP > nrmin) 
    maxP = nrmin - 1;      // <- Change by Joakim; using nrmin here will load to out 
                           // bounds access oc cv_data->Z at line 460.

  


  if ( nrens < cv_data->nfolds )
    util_abort("%s: number of ensemble members %d need to be larger than the number of cv-folds - aborting \n",
               __func__,
               nrens,
               cv_data->nfolds); 
  
  for (int i=0; i < nrens; i++)
    randperms[i] = i;

  rng_shuffle_int( cv_data->rng , randperms , nrens );
  
  cvError = matrix_alloc( maxP , cv_data->nfolds );
  {
    int ntest, ntrain, k,j,i;
    int * indexTest  = util_calloc( nrens , sizeof * indexTest  );
    int * indexTrain = util_calloc( nrens , sizeof * indexTrain );
    for (i = 0; i < cv_data->nfolds; i++) {
      ntest = 0;
      ntrain = 0;
      k = i;
      /*extract members for the training and test ensembles */
      for (j = 0; j < nrens; j++) {
        if (j == k) {
          indexTest[ntest] = randperms[j];
          k += cv_data->nfolds;
          ntest++;
        } else {
          indexTrain[ntrain] = randperms[j];
          ntrain++;
        }
      }

      /*Perform CV for each subspace dimension p */
      cv_enkf_get_cv_error_prin_comp( cv_data , cvError , A , indexTest , indexTrain, ntest, ntrain , i , maxP);
    }
    free( indexTest );
    free( indexTrain );
  }
  

  /* find optimal truncation value for the cv-scheme */
  optP = cv_enkf_get_optimal_numb_comp( cv_data , cvError , maxP);

  matrix_free( cvError );
  free( randperms );
  return optP;
}



/*NB! HERE WE COUNT optP from 0,1,2,... */
static void getW_prin_comp(cv_enkf_data_type * cv_data , matrix_type *W , const int optP) { 

  int i, j;
  double tmp2;
  int nrens = matrix_get_columns( cv_data->Z );
  
  /* Finally, compute W = Z(1:p,:)' * inv(Z(1:p,:) * Z(1:p,:)' + (n -1) * Rp) */
  matrix_type *Zp    = matrix_alloc( optP, nrens );
  matrix_type *SigZp = matrix_alloc( optP ,optP);

  // This loop will fail with i to large whcn accessing cv_data->Z
  // if we do not limit maxP to nrmin - 1?
  for (i = 0; i < optP ; i++) 
    for (j = 0; j < nrens; j++) 
      matrix_iset_safe(Zp , i , j , matrix_iget_safe(cv_data->Z , i ,j));

  // Matrix copy_block should be used in stead of the double (i,j) loop;
  // however that failed because the cv_data->Z matrix had one row too few?
  // matrix_copy_block( Zp , 0 , 0 , optP , nrens , cv_data->Z , 0 , 0 );

  /*Compute SigZp = Zp * Zp' */
  matrix_dgemm( SigZp , Zp , Zp, false , true , 1.0, 0.0);
  
  /*Add (ntrain-1) * Rp*/
  for(i = 0; i < optP; i++) {
    for( j = 0; j < optP; j++) {
      tmp2 = matrix_iget(SigZp , i , j) + (nrens - 1) * matrix_iget(cv_data->Rp, i, j);
      matrix_iset( SigZp , i , j , tmp2 );
    }
  }
  

  /* Invert the covariance matrix for the principal components  */
  int inv_ok = matrix_inv( SigZp );
  
  /*Check if the inversion went ok */
  if ( inv_ok != 0 ) 
    util_abort("%s: inversion of covariance matrix for the principal components failed for subspace dimension p = %d\n - aborting \n",__func__,optP); 
  


  
  /*Compute W = Zp' * inv(SigZp) */
  matrix_dgemm( W , Zp , SigZp , true , false , 1.0 , 0.0);

  matrix_free( Zp );
  matrix_free( SigZp );
}




void cv_enkf_initX(void * module_data , 
                   matrix_type * X , 
                   matrix_type * A , 
                   matrix_type * S , 
                   matrix_type * R , 
                   matrix_type * dObs , 
                   matrix_type * E ,
                   matrix_type * D) {

  
  cv_enkf_data_type * cv_data = cv_enkf_data_safe_cast( module_data );
  printf("Running CV\n");
  {
    int optP;
    int ens_size = matrix_get_columns( S );
    
    /* Get the optimal number of principal components 
       where p is found minimizing the PRESS statistic */
        
    {
      matrix_type * workA = matrix_alloc_copy( A ); 
      matrix_subtract_row_mean( workA );
      optP = get_optimal_principal_components(cv_data , workA );
      printf("Optimal subspace dimension found %d\n",optP);
      matrix_free( workA );
    }

    {
      matrix_type * W          = matrix_alloc(ens_size , optP);                      

      /* Compute  W = Z(1:p,:)' * inv(Z(1:p,:) * Z(1:p,:)' + (ens_size-1) * Rp(1:p,1:p))*/
      getW_prin_comp( cv_data , W , optP);
      
      /*Compute the actual X5 matrix: */
      /*Compute X5 = W * Dp (The hard way) */
      for( int i = 0; i < ens_size; i++) {
        for( int j = 0; j < ens_size; j++) {
          double tmp = 0.0;
          
          for( int k = 0; k < optP; k++) 
            tmp += matrix_iget( W , i , k) * matrix_iget( cv_data->Dp , k , j);
          
          matrix_iset(X , i , j ,tmp);
        }
        matrix_iadd( X , i , i , 1.0);  /* X5 = I + X5 */
      }
      
      matrix_free( W );
    }
  }
}



void cv_enkf_complete_update( void * arg ) {
  cv_enkf_data_type * cv_data = cv_enkf_data_safe_cast( arg );
  {
    matrix_free( cv_data->Z  );
    matrix_free( cv_data->Rp );
    matrix_free( cv_data->Dp );

    cv_data->Z  = NULL;
    cv_data->Rp = NULL;
    cv_data->Dp = NULL;
  }
}



bool cv_enkf_set_double( void * arg , const char * var_name , double value) {
  cv_enkf_data_type * module_data = cv_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , ENKF_TRUNCATION_KEY_) == 0)
      cv_enkf_set_truncation( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


bool cv_enkf_set_int( void * arg , const char * var_name , int value) {
  cv_enkf_data_type * module_data = cv_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;
    
    if (strcmp( var_name , ENKF_NCOMP_KEY_) == 0)
      cv_enkf_set_subspace_dimension( module_data , value );
    else if (strcmp( var_name , NFOLDS_KEY) == 0)
      cv_enkf_set_nfolds( module_data , value);
    else
      name_recognized = false;
    
    return name_recognized;
  }
}


bool cv_enkf_set_bool( void * arg , const char * var_name , bool value) {
  cv_enkf_data_type * module_data = cv_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;
    if (strcmp( var_name , CV_PEN_PRESS_KEY) == 0)
      cv_enkf_set_pen_press( module_data , value );
    else
      name_recognized = false;
    
    return name_recognized;
  }
}


long cv_enkf_get_options( void * arg , long flag) {
  cv_enkf_data_type * cv_data = cv_enkf_data_safe_cast( arg );
  {
    return cv_data->option_flags;
  }
}




#ifdef INTERNAL_LINK
#define SYMBOL_TABLE cv_enkf_symbol_table
#else
#define SYMBOL_TABLE EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type SYMBOL_TABLE = {
  .alloc           = cv_enkf_data_alloc,
  .freef           = cv_enkf_data_free,
  .set_int         = cv_enkf_set_int , 
  .set_double      = cv_enkf_set_double , 
  .set_bool        = cv_enkf_set_bool , 
  .set_string      = NULL , 
  .get_options     = cv_enkf_get_options , 
  .initX           = cv_enkf_initX , 
  .updateA         = NULL,
  .init_update     = cv_enkf_init_update , 
  .complete_update = cv_enkf_complete_update ,
  .has_var         = NULL,
  .get_int         = NULL,
  .get_double      = NULL,
  .get_ptr         = NULL, 
};
