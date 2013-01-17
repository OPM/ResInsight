/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_analysis.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <util.h>
#include <math.h>
#include <matrix.h>
#include <matrix_lapack.h>
#include <matrix_blas.h>
#include <meas_data.h>
#include <obs_data.h>
#include <analysis_config.h>
#include <enkf_util.h>
#include <enkf_analysis.h>
#include <timer.h>
#include <rng.h>
#include <analysis_module.h>
#include <enkf_linalg.h>








//CV: int enkf_analysis_get_optimal_numb_comp(const matrix_type * cvErr , const int maxP , const int nFolds , const bool pen_press) {
//CV: 
//CV:   int i, optP;
//CV:   double tmp, minErr;
//CV:   
//CV:   double tmp2 = (1.0 / (double)nFolds); 
//CV: 
//CV:   double * cvMean = util_malloc( sizeof * cvMean * maxP, __func__);
//CV:   
//CV:   for (int p = 0; p < maxP; p++ ){
//CV:     tmp = 0.0;
//CV:     for (int folds = 0; folds < nFolds; folds++ ){
//CV:       tmp += matrix_iget( cvErr , p, folds );
//CV:     }
//CV:     cvMean[p] = tmp * tmp2;
//CV:   }
//CV: 
//CV:   
//CV:   tmp2 = 1.0 / ((double)(nFolds - 1));
//CV:   double * cvStd = util_malloc( sizeof * cvStd * maxP, __func__);
//CV:   for ( int p = 0; p < maxP; p++){
//CV:     tmp = 0.0;
//CV:     for ( int folds = 0; folds < nFolds; folds++){
//CV:       tmp += pow( matrix_iget( cvErr , p , folds ) - cvMean[p] , 2);
//CV:     }
//CV:     cvStd[p] = sqrt( tmp * tmp2 );
//CV:   }
//CV: 
//CV:   minErr = cvMean[0];
//CV:   optP = 1;
//CV:   
//CV: 
//CV:   printf("PRESS = \n");
//CV:   for (i = 0; i < maxP; i++) {
//CV:     printf(" %0.2f \n",cvMean[i]);
//CV:   }
//CV:   
//CV: 
//CV: 
//CV:   for (i = 1; i < maxP; i++) {
//CV:     tmp = cvMean[i];
//CV:     if (tmp < minErr && tmp > 0.0) {
//CV:       minErr = tmp;
//CV:       optP = i+1;
//CV:     }
//CV:   }
//CV: 
//CV:   printf("Global optimum= %d\n",optP);
//CV:   
//CV: 
//CV:   if (pen_press) {
//CV:     printf("Selecting optimal number of components using Penalised PRESS statistic: \n");
//CV:     for ( i = 0; i < optP; i++){
//CV:       if( cvMean[i] - cvStd[i] <= minErr ){
//CV:         optP = i+1;
//CV:         break;
//CV:       }
//CV:     }
//CV:   }
//CV:   
//CV: 
//CV:   free( cvStd );
//CV:   free( cvMean );
//CV:   return optP;
//CV: }

//CV ->  
//CV ->
//CV ->/* function that estimates the Predictive Error Sum of Squares (PRESS)
//CV ->   statistic based on k-fold Cross-Validation for a particular set of
//CV ->   training and test indcies. Note that we do not need to recompute
//CV ->   the eigenvalue decomposition X0 in equation 14.26 (Evensen, 2007)
//CV ->   for each value of p.
//CV ->
//CV ->   OUTPUT :
//CV ->
//CV ->   cvErr      -  Vector containing the estimate PRESS for all valid combinations of p in the prediction
//CV ->
//CV ->   INPUT :
//CV ->
//CV ->   A          -  Matrix constaining the state vector ensemble
//CV ->   VT         -  Matrix containing the transpose of the right singular vector of the S matrix 
//CV ->   Z          -  Matrix containing the eigenvalues of the X0 matrix defined in Eq. 14.26 in Evensen (2007)
//CV ->   eig        -  Vector containing the diagonal elements of the matrix L1i = inv(I + L1), where L1 
//CV ->   are the eigenvalues of the X0 matrix above
//CV ->   indexTest  -  Vector containing integers specifying which ensemble members are
//CV ->   contained in the test ensemble
//CV ->   indexTrain -  Vector containing integers specifying which ensemble members are 
//CV ->   contained in the training ensemble               
//CV ->   nTest      -  Number of ensemble members in the test ensemble
//CV ->   nTrain     -  Number of ensemble members in the training ensemble
//CV ->*/
//CV ->
//CV ->
//CV -> 

//CV: static void enkf_analysis_get_cv_error(matrix_type * cvErr , 
//CV:                                        const matrix_type * A , 
//CV:                                        const matrix_type * VT , 
//CV:                                        const matrix_type * Z , 
//CV:                                        const double * eig, 
//CV:                                        const int * indexTest, 
//CV:                                        const int * indexTrain , 
//CV:                                        const int nTest , 
//CV:                                        const int nTrain , 
//CV:                                        const int foldIndex) { 
//CV:   /*  
//CV:       We need to predict ATest(p), for p = 1,...,nens -1, based on the estimated regression model:
//CV:       ATest(p) = A[:,indexTrain] * VT[1:p,indexTrain]' * Z[1:p,1:p] * eig[1:p,1:p] * Z[1:p,1:p]' * VT[1:p,testIndex]
//CV:   */
//CV:   const int nx   = matrix_get_rows( A );
//CV: 
//CV:   matrix_type * AHat   = matrix_alloc(nx , nTest );
//CV:   matrix_type * W3     = matrix_alloc(nTrain, nTest );
//CV:   matrix_type * ATrain = matrix_alloc( nx , nTrain );
//CV:   
//CV:   int p,i,j;
//CV:   double tmp, tmp2;
//CV: 
//CV:   int maxP = matrix_get_rows( VT );
//CV:   
//CV:   /* We only want to search the non-zero eigenvalues */
//CV:   for (i = 0; i < maxP; i++) {
//CV:     if (eig[i] == 1.0) {
//CV:       maxP = i;
//CV:       break;
//CV:     }
//CV:   }
//CV: 
//CV:   
//CV: 
//CV: 
//CV:   /* Copy elements*/
//CV:   for (i = 0; i < nx; i++) 
//CV:     for (j = 0; j < nTrain; j++) 
//CV:       matrix_iset(ATrain , i , j , matrix_iget( A , i , indexTrain[j]));
//CV:   
//CV:   for (p = 0; p < maxP; p++) {
//CV:     matrix_type * W = matrix_alloc(p + 1 , nTest );
//CV:     matrix_type * W2 = matrix_alloc(p + 1 , nTest );
//CV:     
//CV:     /* Matrix multiplication: W = Z[1:p,1:p]' * VT[1:p,indexTest] */
//CV:     for (i = 0; i < p; i++) {
//CV:       for (j = 0; j < nTest; j++) {
//CV:         double ksum = 0;
//CV:         for (int k = 0; k < p; k++) 
//CV:           ksum += matrix_iget(Z , k , i) * matrix_iget(VT , k , indexTest[j]);
//CV:         
//CV:         matrix_iset(W , i , j , ksum);
//CV:       }
//CV:     }
//CV:      
//CV:     /*Multiply W with the diagonal matrix eig[1:p,1:p] from the left */
//CV:     for (j=0; j < nTest; j++)
//CV:       for (i=0; i < p; i++)
//CV:         matrix_imul(W , i , j , eig[i]);
//CV:     
//CV:     for (i = 0; i < p; i++) {
//CV:       for (j = 0; j < nTest; j++) {
//CV:         double ksum = 0;
//CV:         for (int k = 0; k < p; k++) 
//CV:           ksum += matrix_iget(Z , i , k) * matrix_iget(W , k , j);
//CV:         
//CV:         matrix_iset(W2 , i , j , ksum);
//CV:       }
//CV:     }
//CV:     matrix_free( W );
//CV: 
//CV:     
//CV:     /*Compute W3 = VT[TrainIndex,1:p]' * W*/
//CV:     for (i = 0; i < nTrain; i++) {
//CV:       for (j = 0; j < nTest; j++) {
//CV:         tmp = 0.0;
//CV:         for (int k = 0; k < p; k++) {
//CV:           tmp += matrix_iget(VT , k , indexTrain[i] ) * matrix_iget(W2 , k , j);
//CV:         }
//CV:           
//CV:         matrix_iset(W3 , i , j , tmp);
//CV:       }
//CV:     }
//CV: 
//CV:     matrix_free( W2 );
//CV:     
//CV: 
//CV:     matrix_matmul(AHat , ATrain , W3 );
//CV: 
//CV: 
//CV: 
//CV:     /*Compute Press Statistic: */
//CV:     tmp = 0.0;
//CV:     
//CV:     for (i = 0; i < nx; i++) {
//CV:       for (j = 0; j < nTest; j++) {
//CV:         tmp2 = matrix_iget(A , i , indexTest[j]) - matrix_iget(AHat , i , j);
//CV:         tmp += tmp2 * tmp2;
//CV:       }
//CV:     }
//CV:     
//CV:     matrix_iset( cvErr , p , foldIndex , tmp );
//CV:     
//CV:   } /*end for p */
//CV:   
//CV:   matrix_free( AHat );
//CV:   matrix_free( ATrain );
//CV:   matrix_free( W3 );
//CV: 
//CV: }


//CV ->
//CV ->
//CV ->/*Function that computes the PRESS for different subspace dimensions using
//CV ->  m-fold CV 
//CV ->  INPUT :
//CV ->  A   : State-Vector ensemble matrix
//CV ->  Z   : Ensemble matrix of principal components
//CV ->  Rp  : Reduced order Observation error matrix
//CV ->  indexTrain: index of training ensemble
//CV ->  indexTest: index of test ensemble
//CV ->  nTest : number of members in the training ensemble
//CV ->  nTrain . number of members in the test ensemble
//CV ->  foldIndex: integer specifying which "cv-fold" we are considering
//CV ->  
//CV ->  OUTPUT:
//CV ->  cvErr : UPDATED MATRIX OF PRESS VALUES
//CV ->  
//CV ->*/
//CV ->static void enkf_analysis_get_cv_error_prin_comp(matrix_type * cvErr , const matrix_type * A , const matrix_type * Z , const matrix_type * Rp , const int * indexTest, const int * indexTrain , const int nTest , const int nTrain , const int foldIndex, const int maxP) { 
//CV ->  /*  We need to predict ATest(p), for p = 1,...,nens -1, based on the estimated regression model:
//CV ->      AHatTest(p) = A[:,indexTrain] * Z[1:p,indexTrain]'* inv( Z[1:p,indexTrain] * Z[1:p,indexTrain]' + (nens-1) * Rp[1:p,1:p] ) * Z[1:p,indexTest];
//CV ->  */
//CV ->  
//CV ->  /* Start by multiplying from the right: */
//CV ->  int p,i,j,k, inv_ok, tmp3;
//CV ->  double tmp, tmp2;
//CV ->
//CV ->
//CV ->
//CV ->  const int nx   = matrix_get_rows( A );
//CV ->
//CV ->
//CV ->  matrix_type * AHat = matrix_alloc(nx , nTest );
//CV ->    
//CV ->  /*We want to use the blas function to speed things up: */
//CV ->  matrix_type * ATrain = matrix_alloc( nx , nTrain );
//CV ->  /* Copy elements*/
//CV ->  for (i = 0; i < nx; i++) {
//CV ->    for (j = 0; j < nTrain; j++) {
//CV ->      matrix_iset(ATrain , i , j , matrix_iget( A , i , indexTrain[j]));
//CV ->    }
//CV ->  }
//CV ->  
//CV ->  tmp3 = nTrain - 1;
//CV ->  int pOrg;
//CV ->
//CV ->  for (p = 0; p < maxP; p++) {
//CV ->
//CV ->    pOrg = p + 1;
//CV ->
//CV ->
//CV ->    /*For now we do this the hard way through a full inversion of the reduced data covariance matrix: */
//CV ->    /* Alloc ZTrain(1:p): */
//CV ->    matrix_type *ZpTrain = matrix_alloc( pOrg, nTrain );
//CV ->    for (i = 0; i < pOrg ; i++) {
//CV ->      for (j = 0; j < nTrain; j++) {
//CV ->        matrix_iset(ZpTrain , i , j , matrix_iget(Z , i ,indexTrain[j]));
//CV ->      }
//CV ->    }
//CV ->
//CV ->    matrix_type *SigDp = matrix_alloc( pOrg ,pOrg);
//CV ->    /*Compute SigDp = ZpTrain * ZpTrain' */
//CV ->    matrix_dgemm( SigDp , ZpTrain , ZpTrain, false , true , 1.0, 0.0);
//CV ->    
//CV ->    /*Add (ntrain-1) * Rp*/
//CV ->
//CV ->    for(i = 0; i < pOrg; i++) {
//CV ->      for( j = 0; j < pOrg; j++) {
//CV ->        tmp2 = matrix_iget(SigDp , i , j) + tmp3 * matrix_iget(Rp, i, j);
//CV ->        matrix_iset( SigDp , i , j , tmp2 );
//CV ->      }
//CV ->    }
//CV ->    
//CV ->    /* Invert the covariance matrix for the principal components  */
//CV ->    inv_ok = matrix_inv( SigDp );
//CV ->
//CV ->    
//CV ->    /*Check if the inversion went ok */
//CV ->    if ( inv_ok != 0 ) {
//CV ->      util_abort("%s: inversion of covariance matrix for the principal components failed for subspace dimension p = %d\n - aborting \n",__func__,pOrg); 
//CV ->    }
//CV ->
//CV ->    
//CV ->    /*Compute inv(SigDp) * ZTest: */
//CV ->    matrix_type * W = matrix_alloc(pOrg , nTest );
//CV ->    for (i = 0; i < pOrg; i++) {
//CV ->      for (j = 0; j < nTest; j++) {
//CV ->        tmp = 0.0;
//CV ->        for (k = 0; k < pOrg; k++) {
//CV ->          tmp += matrix_iget(SigDp , i , k) * matrix_iget(Z , k , indexTest[j]);
//CV ->        }
//CV ->        matrix_iset(W , i , j , tmp);
//CV ->      }
//CV ->    }
//CV ->
//CV ->
//CV ->
//CV ->    matrix_type * W2 = matrix_alloc(nTrain , nTest );
//CV ->    /*Compute W2 = ZpTrain' * W */
//CV ->    matrix_dgemm( W2 , ZpTrain , W , true , false , 1.0 , 0.0);
//CV ->    
//CV ->    matrix_free( ZpTrain );
//CV ->    matrix_free( SigDp );
//CV ->    matrix_free( W );
//CV ->
//CV ->    /*Estimate the state-vector */
//CV ->    matrix_matmul(AHat , ATrain , W2 );
//CV ->    matrix_free( W2 );
//CV ->
//CV ->    /*Compute Press Statistic: */
//CV ->    tmp = 0.0;
//CV ->    
//CV ->    for (i = 0; i < nx; i++) {
//CV ->      for (j = 0; j < nTest; j++) {
//CV ->        tmp2 = matrix_iget(A , i , indexTest[j]) - matrix_iget(AHat , i , j);
//CV ->        tmp += tmp2 * tmp2;
//CV ->      }
//CV ->    }
//CV ->    
//CV ->    matrix_iset( cvErr , p , foldIndex , tmp );
//CV ->    
//CV ->  } /*end for p */
//CV ->  
//CV ->  matrix_free( AHat );
//CV ->  matrix_free( ATrain );
//CV ->}
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->/*Special function for doing cross-validation */ 
//CV ->static void getW_pre_cv(matrix_type * W , const matrix_type * V0T, const matrix_type * Z , double * eig , const matrix_type * U0 , int nfolds_CV, 
//CV ->                        const matrix_type * A, int unique_bootstrap_components , rng_type * rng, bool pen_press) {
//CV ->
//CV ->  const int nrobs = matrix_get_rows( U0 );
//CV ->  const int nrens = matrix_get_columns( V0T );
//CV ->  const int nrmin = util_int_min( nrobs , nrens );
//CV ->
//CV ->  int i,j;
//CV ->  
//CV -> 
//CV ->  /* Vector with random permutations of the itegers 1,...,nrens  */
//CV ->  int * randperms     = util_malloc( sizeof * randperms * nrens, __func__);
//CV ->  int * indexTest     = util_malloc( sizeof * indexTest * nrens, __func__);
//CV ->  int * indexTrain    = util_malloc( sizeof * indexTrain * nrens, __func__);
//CV ->
//CV ->  if(nrens != unique_bootstrap_components)
//CV ->    nfolds_CV = util_int_min( nfolds_CV , unique_bootstrap_components-1);
//CV ->
//CV ->
//CV ->  matrix_type * cvError = matrix_alloc( nrmin,nfolds_CV );
//CV ->  
//CV ->  /*Copy Z */
//CV ->  matrix_type * workZ = matrix_alloc_copy( Z );
//CV ->
//CV ->  int optP;
//CV ->  
//CV -> 
//CV ->  /* start cross-validation: */
//CV ->
//CV ->  const int maxp = matrix_get_rows(V0T);
//CV ->  
//CV ->  /* draw random permutations of the integers 1,...,nrens */  
//CV ->  enkf_util_randperm( randperms , nrens , rng);
//CV ->  
//CV ->  /*need to init cvError to all zeros (?) */
//CV ->  for (i = 0; i < nrmin; i++){
//CV ->    for( j = 0; j> nfolds_CV; j++){
//CV ->      matrix_iset( cvError , i , j , 0.0 );
//CV ->    }
//CV ->  }
//CV ->  
//CV ->  int ntest, ntrain, k;
//CV ->  printf("\nStarting cross-validation\n");
//CV ->  for (i = 0; i < nfolds_CV; i++) {
//CV ->    printf(".");
//CV ->
//CV ->    ntest = 0;
//CV ->    ntrain = 0;
//CV ->    k = i;
//CV ->    /*extract members for the training and test ensembles */
//CV ->    for (j = 0; j < nrens; j++) {
//CV ->      if (j == k) {
//CV ->        indexTest[ntest] = randperms[j];
//CV ->        k += nfolds_CV;
//CV ->        ntest++;
//CV ->      } else {
//CV ->        indexTrain[ntrain] = randperms[j];
//CV ->        ntrain++;
//CV ->      }
//CV ->    }
//CV ->    enkf_analysis_get_cv_error( cvError , A , V0T , workZ , eig , indexTest , indexTrain, ntest, ntrain , i );
//CV ->  }
//CV ->  printf("\n");
//CV ->  /* find optimal truncation value for the cv-scheme */
//CV ->  optP = enkf_analysis_get_optimal_numb_comp( cvError , maxp, nfolds_CV, pen_press);
//CV ->
//CV ->  printf("Optimal number of components found: %d \n",optP);
//CV ->  printf("\n");
//CV ->  FILE * compSel_log = util_fopen("compSel_log_local_cv" , "a");
//CV ->  fprintf( compSel_log , " %d ",optP);
//CV ->  fclose( compSel_log);
//CV ->
//CV ->
//CV ->  /*free cvError vector and randperm */
//CV ->  matrix_free( cvError );
//CV ->  free( randperms );
//CV ->  free( indexTest );
//CV ->  free( indexTrain );
//CV ->
//CV ->  /* need to update matrices so that we only use components 1,...,optP */
//CV ->  /* remove non-zero entries of the z matrix (we do not want to recompute sigma0^(+') * z */
//CV ->  /* this can surely be done much more efficiently, but for now we want to minimize the
//CV ->     number of potential bugs in the code for now */ 
//CV ->  for (i = optP; i < nrmin; i++) {
//CV ->    for (j = 0; j < nrmin; j++) {
//CV ->      matrix_iset(workZ , i , j, 0.0);
//CV ->    }
//CV ->  }
//CV ->  
//CV ->    
//CV ->  /*fix the eig vector as well: */
//CV ->  {
//CV ->    int i;
//CV ->    /* lambda1 = (i + lambda)^(-1) */
//CV ->    for (i=optP; i < nrmin; i++) 
//CV ->      eig[i] = 1.0;
//CV ->  }
//CV ->
//CV ->  matrix_matmul(W , U0 , workZ); /* x1 = w = u0 * z2 = u0 * sigma0^(+') * z    */
//CV ->  
//CV ->
//CV ->
//CV ->
//CV ->  /*end cross-validation */
//CV ->}
//CV ->
//CV ->
//CV ->/* Function that performs cross-validation to find the optimal subspace dimension,  */
//CV ->
//CV ->
//CV ->int get_optimal_principal_components(const matrix_type * Z , const matrix_type * Rp , int nfolds_CV, const matrix_type * A, rng_type * rng, const int maxP, bool pen_press) {
//CV ->
//CV ->  const int nrens = matrix_get_columns( Z );
//CV ->  const int nrmin = matrix_get_rows( Z );
//CV ->
//CV ->  int i,j;
//CV ->  
//CV -> 
//CV ->  /* Vector with random permutations of the itegers 1,...,nrens  */
//CV ->  int * randperms     = util_malloc( sizeof * randperms * nrens, __func__);
//CV ->  int * indexTest     = util_malloc( sizeof * indexTest * nrens, __func__);
//CV ->  int * indexTrain    = util_malloc( sizeof * indexTrain * nrens, __func__);
//CV ->
//CV ->
//CV ->  if ( nrens < nfolds_CV )
//CV ->    util_abort("%s: number of ensemble members %d need to be larger than the number of cv-folds - aborting \n",__func__,nrens,nfolds_CV); 
//CV ->  
//CV ->  
//CV ->  
//CV ->  int optP;
//CV ->
//CV ->
//CV ->  printf("\nOnly searching for the optimal subspace dimension among the first %d principal components\n",maxP);
//CV ->  
//CV ->  matrix_type * cvError = matrix_alloc( maxP ,nfolds_CV );
//CV ->
//CV -> 
//CV ->  /* start cross-validation: */
//CV ->  if ( nrens < nfolds_CV )
//CV ->    util_abort("%s: number of ensemble members %d need to be larger than the number of cv-folds - aborting \n",__func__,nrens,nfolds_CV); 
//CV ->
//CV ->  
//CV ->  /* draw random permutations of the integers 1,...,nrens */  
//CV ->  enkf_util_randperm( randperms , nrens , rng);
//CV ->  
//CV ->  /*need to init cvError to all zeros (?) */
//CV ->  for (i = 0; i < nrmin; i++){
//CV ->    for( j = 0; j> nfolds_CV; j++){
//CV ->      matrix_iset( cvError , i , j , 0.0 );
//CV ->    }
//CV ->  }
//CV ->  
//CV ->  int ntest, ntrain, k;
//CV ->  printf("Starting cross-validation\n");
//CV ->  for (i = 0; i < nfolds_CV; i++) {
//CV ->    printf(".");
//CV ->
//CV ->    ntest = 0;
//CV ->    ntrain = 0;
//CV ->    k = i;
//CV ->    /*extract members for the training and test ensembles */
//CV ->    for (j = 0; j < nrens; j++) {
//CV ->      if (j == k) {
//CV ->        indexTest[ntest] = randperms[j];
//CV ->        k += nfolds_CV;
//CV ->        ntest++;
//CV ->      } else {
//CV ->        indexTrain[ntrain] = randperms[j];
//CV ->        ntrain++;
//CV ->      }
//CV ->    }
//CV ->    
//CV ->    /*Perform CV for each subspace dimension p */
//CV ->    enkf_analysis_get_cv_error_prin_comp( cvError , A , Z , Rp , indexTest , indexTrain, ntest, ntrain , i , maxP);
//CV ->  }
//CV ->  printf("\n");
//CV ->  /* find optimal truncation value for the cv-scheme */
//CV ->  optP = enkf_analysis_get_optimal_numb_comp( cvError , maxP, nfolds_CV , pen_press);
//CV ->
//CV ->  printf("Optimal number of components found: %d \n",optP);
//CV ->  FILE * compSel_log = util_fopen("compSel_log_local_cv" , "a");
//CV ->  fprintf( compSel_log , " %d ",optP);
//CV ->  fclose( compSel_log);
//CV ->
//CV ->
//CV ->  /*free cvError vector and randperm */
//CV ->  matrix_free( cvError );
//CV ->  free( randperms );
//CV ->  free( indexTest );
//CV ->  free( indexTrain );
//CV ->
//CV ->
//CV ->  return optP;
//CV ->}
//CV ->
//CV ->
//CV ->/*NB! HERE WE COUNT optP from 0,1,2,... */
//CV ->static void getW_prin_comp(matrix_type *W , const matrix_type * Z , 
//CV ->                           const matrix_type * Rp , const int optP) { 
//CV ->
//CV ->  int i, j;
//CV ->  double tmp2;
//CV ->  int nrens = matrix_get_columns( Z );
//CV ->  
//CV ->  /* Finally, compute W = Z(1:p,:)' * inv(Z(1:p,:) * Z(1:p,:)' + (n -1) * Rp) */
//CV ->  matrix_type *Zp = matrix_alloc( optP, nrens );
//CV ->  for (i = 0; i < optP ; i++) {
//CV ->    for (j = 0; j < nrens; j++) {
//CV ->      matrix_iset(Zp , i , j , matrix_iget(Z , i ,j));
//CV ->    }
//CV ->  }
//CV ->
//CV ->  matrix_type *SigZp = matrix_alloc( optP ,optP);
//CV ->  /*Compute SigZp = Zp * Zp' */
//CV ->  matrix_dgemm( SigZp , Zp , Zp, false , true , 1.0, 0.0);
//CV ->  
//CV ->  /*Add (ntrain-1) * Rp*/
//CV ->  
//CV ->  int tmp3 = nrens - 1;
//CV ->
//CV ->
//CV ->  for(i = 0; i < optP; i++) {
//CV ->    for( j = 0; j < optP; j++) {
//CV ->      tmp2 = matrix_iget(SigZp , i , j) + tmp3 * matrix_iget(Rp, i, j);
//CV ->      matrix_iset( SigZp , i , j , tmp2 );
//CV ->    }
//CV ->  }
//CV ->  
//CV ->  /* Invert the covariance matrix for the principal components  */
//CV ->  int inv_ok = matrix_inv( SigZp );
//CV ->  
//CV ->  /*Check if the inversion went ok */
//CV ->  if ( inv_ok != 0 ) {
//CV ->    util_abort("%s: inversion of covariance matrix for the principal components failed for subspace dimension p = %d\n - aborting \n",__func__,optP); 
//CV ->  }
//CV ->  
//CV ->
//CV ->
//CV ->  
//CV ->  /*Compute W = Zp' * inv(SigZp) */
//CV ->  matrix_dgemm( W , Zp , SigZp , true , false , 1.0 , 0.0);
//CV ->
//CV ->  matrix_free( Zp );
//CV ->  matrix_free( SigZp );
//CV ->  
//CV ->
//CV ->}



  










/*****************************************************************/
/*****************************************************************/
/*                     High level functions                      */
/*****************************************************************/
/*****************************************************************/


//void enkf_analysis_invertS(const analysis_config_type * config , const matrix_type * S , const matrix_type * R , matrix_type * W , double * eig) {
//  pseudo_inversion_type inversion_mode  = analysis_config_get_inversion_mode( config );  
//  bool force_subspace_dimension         = analysis_config_get_force_subspace_dimension( config );
//  int  ens_size                         = matrix_get_columns( S );  
//  int  nrobs                            = matrix_get_rows( S ); 
//  double truncation                     = -1;
//  int    ncomp                          = -1;
//
//  if (force_subspace_dimension) {
//    ncomp = analysis_config_get_subspace_dimension( config );
//    /* Check if the dimension is appropropriate. If not, change to default */
//    if (ncomp > util_int_min( ens_size - 1, nrobs) ) {
//      printf("Selected number of components, %d, too high. Changing to default value: 1\n",ncomp);
//      ncomp = 1;
//    }
//  } else
//    truncation = analysis_config_get_truncation( config );
//  
//  switch (inversion_mode) {
//  case(SVD_SS_N1_R):
//    enkf_linalg_lowrankCinv( S , R , W , eig , truncation , ncomp);    
//    break;
//  default:
//    util_abort("%s: inversion mode:%d not supported \n",__func__ , inversion_mode);
//  }
//}
//
//
//
//void enkf_analysis_invertS_pre_cv(const analysis_config_type * config , 
//                                  const matrix_type * S , 
//                                  const matrix_type * R , 
//                                  matrix_type * V0T , 
//                                  matrix_type * Z , 
//                                  double * eig , 
//                                  matrix_type * U0 ) {
//
//  pseudo_inversion_type inversion_mode  = analysis_config_get_inversion_mode( config );  
//  double truncation                     = analysis_config_get_truncation( config );  
//  
//  switch (inversion_mode) {
//  case(SVD_SS_N1_R):
//    enkf_linalg_lowrankCinv__( S , R , V0T , Z , eig , U0 , truncation , -1);    
//    break;
//  default:
//    util_abort("%s: inversion mode:%d not supported \n",__func__ , inversion_mode);
//  }
//}
//
//
//
//
//
//static void enkf_analysis_standard(matrix_type * X5 , const matrix_type * S , const matrix_type * D , const matrix_type * W , const double * eig, bool bootstrap) {
//  const int nrobs   = matrix_get_rows( S );
//  const int nrens   = matrix_get_columns( S );
//  matrix_type * X3  = matrix_alloc(nrobs , nrens);
//  
//  enkf_linalg_genX3(X3 , W , D , eig ); /*  X2 = diag(eig) * W' * D (Eq. 14.31, Evensen (2007)) */
//                                        /*  X3 = W * X2 = X1 * X2 (Eq. 14.31, Evensen (2007)) */  
//
//  matrix_dgemm( X5 , S , X3 , true , false , 1.0 , 0.0);  /* X5 = S' * X3 */
//  if (!bootstrap) {
//    for (int i = 0; i < nrens; i++)
//      matrix_iadd( X5 , i , i , 1.0); /*X5 = I + X5 */
//  }
//  
//  matrix_free( X3 );
//}
//
//
//static void enkf_analysis_SQRT(matrix_type * X5      , 
//                               const matrix_type * S , 
//                               const matrix_type * randrot , 
//                               const matrix_type * innov , 
//                               const matrix_type * W , 
//                               const double * eig , 
//                               bool bootstrap) {
//
//  const int nrobs   = matrix_get_rows( S );
//  const int nrens   = matrix_get_columns( S );
//  const int nrmin   = util_int_min( nrobs , nrens );
//  
//  matrix_type * X2    = matrix_alloc(nrmin , nrens);
//  
//  if (bootstrap)
//    util_exit("%s: Sorry bootstrap support not fully implemented for SQRT scheme\n",__func__);
//
//  enkf_linalg_meanX5( S , W , eig , innov , X5 );
//  enkf_linalg_genX2(X2 , S , W , eig);
//  enkf_linalg_X5sqrt(X2 , X5 , randrot , nrobs);
//
//  matrix_free( X2 );
//}




/*****************************************************************/

void enkf_analysis_fprintf_obs_summary(const obs_data_type * obs_data , const meas_data_type * meas_data , const int_vector_type * step_list , const char * ministep_name , FILE * stream ) {
  const char * float_fmt = "%15.3f";
  fprintf(stream , "===============================================================================================================================\n");
  fprintf(stream , "Report step...: %04d",int_vector_iget( step_list , 0));
  if (int_vector_size( step_list ) == 1)
    fprintf(stream , "\n");
  else
    fprintf(stream , " - %04d \n",int_vector_get_last( step_list ));
  
  
  fprintf(stream , "Ministep......: %s   \n",ministep_name);  
  fprintf(stream , "-------------------------------------------------------------------------------------------------------------------------------\n");
  {
    char * obs_fmt = util_alloc_sprintf("  %%-3d : %%-32s %s +/-  %s" , float_fmt , float_fmt);
    char * sim_fmt = util_alloc_sprintf("   %s +/- %s  \n"            , float_fmt , float_fmt);

    fprintf(stream , "                                                         Observed history               |             Simulated data        \n");  
    fprintf(stream , "-------------------------------------------------------------------------------------------------------------------------------\n");
    
    {
      int block_nr;
      int obs_count = 1;  /* Only for printing */
      for (block_nr =0; block_nr < obs_data_get_num_blocks( obs_data ); block_nr++) {
        const obs_block_type  * obs_block  = obs_data_iget_block_const( obs_data , block_nr);
        const meas_block_type * meas_block = meas_data_iget_block_const( meas_data , block_nr );
        const char * obs_key = obs_block_get_key( obs_block );
        
        for (int iobs = 0; iobs < obs_block_get_size( obs_block ); iobs++) {
          const char * print_key;
          if (iobs == 0)
            print_key = obs_key;
          else
            print_key = "  ...";
          
          fprintf(stream , obs_fmt , obs_count , print_key , obs_block_iget_value( obs_block , iobs ) , obs_block_iget_std( obs_block , iobs ));
          {
            active_type active_mode = obs_block_iget_active_mode( obs_block , iobs );
            if (active_mode == ACTIVE)
              fprintf(stream , "  Active   |");
            else if (active_mode == DEACTIVATED)
              fprintf(stream , "  Inactive |");
            else if (active_mode == MISSING)
              fprintf(stream , "           |");
            else
              util_abort("%s: enum_value:%d not handled - internal error\n" , __func__ , active_mode);
            if (active_mode == MISSING)
              fprintf(stream , "                  Missing\n");
            else
              fprintf(stream , sim_fmt, meas_block_iget_ens_mean( meas_block , iobs ) , meas_block_iget_ens_std( meas_block , iobs ));
          }
          obs_count++;
        }
      }
    }
    
    free( obs_fmt );
    free( sim_fmt );
  } 
  fprintf(stream , "===============================================================================================================================\n");
  fprintf(stream , "\n\n\n");
}




void enkf_analysis_deactivate_outliers(obs_data_type * obs_data , meas_data_type * meas_data , double std_cutoff , double alpha) {
  for (int block_nr =0; block_nr < obs_data_get_num_blocks( obs_data ); block_nr++) {
    obs_block_type  * obs_block  = obs_data_iget_block( obs_data , block_nr);
    meas_block_type * meas_block = meas_data_iget_block( meas_data , block_nr );
    
    meas_block_calculate_ens_stats( meas_block );
    {
      int iobs;
      for (iobs =0; iobs < meas_block_get_total_size( meas_block ); iobs++) {
        if (meas_block_iget_active( meas_block , iobs )) {
          double ens_std  = meas_block_iget_ens_std( meas_block , iobs );
          if (ens_std < std_cutoff) {
            /*
              De activated because the ensemble has to small
              variation for this particular measurement.
            */
            obs_block_deactivate( obs_block , iobs , "No ensemble variation");
            meas_block_deactivate( meas_block , iobs );
          } else {
            double ens_mean  = meas_block_iget_ens_mean( meas_block , iobs );
            double obs_std   = obs_block_iget_std( obs_block , iobs );
            double obs_value = obs_block_iget_value( obs_block , iobs );
            double innov     = obs_value - ens_mean;
            
            /* 
               Deactivated because the distance between the observed data
               and the ensemble prediction is to large. Keeping these
               outliers will lead to numerical problems.
            */

            if (fabs( innov ) > alpha * (ens_std + obs_std)) {
              obs_block_deactivate(obs_block , iobs , "No overlap");
              meas_block_deactivate(meas_block , iobs);
            }
          }
        }
      }
    }
  }
}



/*
  This function will allocate and initialize the matrices S,R,D and E
  and also the innovation. The matrices will be scaled with the
  observation error and the mean will be subtracted from the S matrix.
*/

// void enkf_analysis_alloc_matrices( rng_type * rng , 
//                                    const meas_data_type * meas_data , 
//                                    obs_data_type * obs_data , 
//                                    enkf_mode_type enkf_mode , 
//                                    matrix_type ** S , 
//                                    matrix_type ** R , 
//                                    matrix_type ** innov,
//                                    matrix_type ** E ,
//                                    matrix_type ** D , 
//                                    bool scale) {
//   
//   int ens_size              = meas_data_get_ens_size( meas_data );
//   int active_size           = obs_data_get_active_size( obs_data );
// 
//   *S                        = meas_data_allocS( meas_data , active_size );
//   *R                        = obs_data_allocR( obs_data , active_size );
//   *innov                    = obs_data_alloc_innov( obs_data , meas_data , active_size );
//   
//   if (enkf_mode == ENKF_STANDARD) {
//     /* 
//        We are using standard EnKF and need to perturbe the measurements,
//        if we are using the SQRT scheme the E & D matrices are not used.
//     */
//     *E = obs_data_allocE(obs_data , rng , ens_size, active_size );
//     *D = obs_data_allocD(obs_data , *E , *S );
//     
//   } else {
//     *E          = NULL;
//     *D          = NULL;
//   }
// 
//   if (scale)
//     obs_data_scale(obs_data ,  *S , *E , *D , *R , *innov );
//   matrix_subtract_row_mean( *S );  /* Subtracting the ensemble mean */
// }






//Boot: static void enkf_analysis_alloc_matrices_boot( rng_type * rng , 
//Boot:                                                const meas_data_type * meas_data , 
//Boot:                                                obs_data_type * obs_data , 
//Boot:                                                enkf_mode_type enkf_mode , 
//Boot:                                                matrix_type ** S , 
//Boot:                                                matrix_type ** R , 
//Boot:                                                matrix_type ** E ,
//Boot:                                                matrix_type ** D ,
//Boot:                                                const meas_data_type * fasit) {
//Boot: 
//Boot:   int ens_size              = meas_data_get_ens_size( meas_data );
//Boot:   int active_size           = obs_data_get_active_size( obs_data );
//Boot:   
//Boot:   *S                        = meas_data_allocS( meas_data , active_size );
//Boot:   *R                        = obs_data_allocR( obs_data , active_size );
//Boot:   
//Boot:   if (enkf_mode == ENKF_STANDARD) {
//Boot:     matrix_type * fullS       = meas_data_allocS( fasit , active_size );
//Boot:     /* 
//Boot:        We are using standard EnKF and need to perturbe the measurements,
//Boot:        if we are using the SQRT scheme the E & D matrices are not used.
//Boot:     */
//Boot:     *E = obs_data_allocE(obs_data , rng , ens_size, active_size );
//Boot:     *D = obs_data_allocD(obs_data , *E , fullS );
//Boot:     matrix_free( fullS );
//Boot:   } else {
//Boot:     *E          = NULL;
//Boot:     *D          = NULL;
//Boot:   }
//Boot:   
//Boot:   obs_data_scale(obs_data ,  *S , *E , *D , *R , NULL );
//Boot:   matrix_subtract_row_mean( *S );  /* Subtracting the ensemble mean */
//Boot: }










/**
   This function allocates a X matrix for the 

      A' = AX

   EnKF update. Same as above except we do not want a resampled version of the D-matrix so
   for bootstrapping purposes we also need the unsampled meas_data.

*/

// matrix_type * enkf_analysis_allocX_boot( const analysis_config_type * config , 
//                                          rng_type * rng , 
//                                          const meas_data_type * meas_data , 
//                                          obs_data_type * obs_data , 
//                                          const matrix_type * randrot , 
//                                          const meas_data_type * fasit) {
//   
//   int ens_size          = meas_data_get_ens_size( meas_data );
//   matrix_type * X       = matrix_alloc( ens_size , ens_size );
//   matrix_set_name( X , "X");
//   {
//     matrix_type * S , *R , *E , *D ;
//     
//     int nrobs                = obs_data_get_active_size(obs_data);
//     int nrmin                = util_int_min( ens_size , nrobs); 
//     
//     matrix_type * W          = matrix_alloc(nrobs , nrmin);                      
//     double      * eig        = util_malloc( sizeof * eig * nrmin , __func__);    
//     enkf_mode_type enkf_mode = analysis_config_get_enkf_mode( config );    
//     bool bootstrap           = analysis_config_get_bootstrap( config );
//     enkf_analysis_alloc_matrices_boot( rng , meas_data , obs_data , enkf_mode , &S , &R , &E , &D , fasit);
// 
//     /* 
//        2: Diagonalize the S matrix; singular vectors are stored in W
//           and singular values (after some massage) are stored in eig. 
//           W = X1, eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
//     */ 
// 
//     enkf_analysis_invertS( config , S , R , W , eig);
//     
//     /* 
//        3: actually calculating the X matrix. 
//     */
//     switch (enkf_mode) {
//     case(ENKF_STANDARD):
//       enkf_analysis_standard(X , S , D , W , eig , bootstrap);
//       break;
//     case(ENKF_SQRT):
//       //enkf_analysis_SQRT(X , S , randrot , innov , W , eig , bootstrap);
//       break;
//     default:
//       util_abort("%s: INTERNAL ERROR \n",__func__);
//     }
//     
//     matrix_free( W );
//     matrix_free( R );
//     matrix_free( S );
//     free( eig );
//     
//     if (enkf_mode == ENKF_STANDARD) {
//       matrix_free( E );
//       matrix_free( D );
//     }
//     
//     enkf_analysis_checkX(X , bootstrap);
//   }
//   return X;
// }


//CV ->matrix_type * enkf_analysis_allocX_pre_cv( const analysis_config_type * config , 
//CV ->                                           rng_type * rng , 
//CV ->                                           meas_data_type * meas_data , 
//CV ->                                           obs_data_type * obs_data , 
//CV ->                                           const matrix_type * randrot , 
//CV ->                                           const matrix_type * A , 
//CV ->                                           const matrix_type * V0T , 
//CV ->                                           const matrix_type * Z ,
//CV ->                                           const double * eig , 
//CV ->                                           const matrix_type * U0 , 
//CV ->                                           meas_data_type * fasit , 
//CV ->                                           int unique_bootstrap_components) {
//CV ->
//CV ->  int ens_size          = meas_data_get_ens_size( meas_data );
//CV ->  matrix_type * X       = matrix_alloc( ens_size , ens_size );
//CV ->  {
//CV ->    int nrobs                = obs_data_get_active_size(obs_data);
//CV ->    int nrmin                = util_int_min( ens_size , nrobs); 
//CV ->    int nfolds_CV            = analysis_config_get_nfolds_CV( config );
//CV ->    
//CV ->    /*
//CV ->      1: Allocating all matrices
//CV ->    */
//CV ->    /*Need a copy of A, because we need it later */
//CV ->    matrix_type * workA      = matrix_alloc_copy( A );    /* <- This is a massive memory requirement. */
//CV ->    matrix_type * S , *R , *E , *D , *innov;
//CV ->    
//CV ->    matrix_type * W          = matrix_alloc(nrobs , nrmin);                      
//CV ->    enkf_mode_type enkf_mode = analysis_config_get_enkf_mode( config );    
//CV ->    bool bootstrap           = analysis_config_get_bootstrap( config );    
//CV ->    bool penalised_press     = analysis_config_get_penalised_press( config );
//CV ->    
//CV ->    double * workeig    = util_malloc( sizeof * workeig * nrmin , __func__);
//CV ->
//CV ->    enkf_analysis_alloc_matrices_boot( rng , meas_data , obs_data , enkf_mode , &S , &R , &innov , &E , &D , fasit ); /*Using the bootstrap version every time, does mean a bit more data 
//CV ->                                                                                                                           carried through the function, but we avoid duplicating code.*/
//CV ->    /*copy entries in eig:*/
//CV ->    {
//CV ->      int i;
//CV ->      for (i = 0 ; i < nrmin ; i++) 
//CV ->        workeig[i] = eig[i];
//CV ->    }
//CV ->    
//CV ->    /* Subtracting the ensemble mean of the state vector ensemble */
//CV ->    matrix_subtract_row_mean( workA );
//CV ->    
//CV ->    /* 
//CV ->       2: Diagonalize the S matrix; singular vectors are stored in W
//CV ->          and singular values (after some massage) are stored in eig. 
//CV ->    W = X1, eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
//CV ->    */ 
//CV ->
//CV ->    getW_pre_cv(W , V0T , Z , workeig ,  U0 , nfolds_CV , workA , unique_bootstrap_components , rng , penalised_press);
//CV ->    
//CV ->    /* 
//CV ->       3: actually calculating the X matrix. 
//CV ->    */
//CV ->    switch (enkf_mode) {
//CV ->    case(ENKF_STANDARD):
//CV ->      enkf_analysis_standard(X , S , D , W , workeig , bootstrap);
//CV ->      break;
//CV ->    case(ENKF_SQRT):
//CV ->      enkf_analysis_SQRT(X , S , randrot , innov , W , workeig , bootstrap );
//CV ->      break;
//CV ->    default:
//CV ->      util_abort("%s: INTERNAL ERROR \n",__func__);
//CV ->    }
//CV ->    
//CV ->    matrix_free( W );
//CV ->    matrix_free( R );
//CV ->    matrix_free( S );
//CV ->    matrix_free( workA );
//CV ->    matrix_free( innov );
//CV ->    free( workeig );
//CV ->    
//CV ->    if (enkf_mode == ENKF_STANDARD) {
//CV ->      matrix_free( E );
//CV ->      matrix_free( D );
//CV ->    }
//CV ->    
//CV ->    enkf_analysis_checkX(X , bootstrap);
//CV ->  }
//CV ->  return X;
//CV ->}
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->
//CV ->/** 
//CV ->    This function initializes the S matrix and performs svd(S). The
//CV ->    left and right singular vectors and singular values are returned
//CV ->    in U0, V0T and eig respectively.
//CV ->*/
//CV ->
//CV ->void enkf_analysis_local_pre_cv( const analysis_config_type * config , rng_type * rng , meas_data_type * meas_data , obs_data_type * obs_data ,  matrix_type * V0T , matrix_type * Z , double * eig , matrix_type * U0, meas_data_type * fasit ) {
//CV ->  {
//CV ->    matrix_type * S , *R , *E , *D , *innov;
//CV ->    
//CV ->    enkf_mode_type enkf_mode = analysis_config_get_enkf_mode( config );
//CV ->    enkf_analysis_alloc_matrices_boot( rng , meas_data , obs_data , enkf_mode , &S , &R , &innov , &E , &D , fasit ); /*Using the bootstrap version every time, does mean a bit more data 
//CV ->                                                                                                                           carried through the function, but we avoid duplicating code.*/
//CV ->    /* 
//CV ->       2: Diagonalize the S matrix; singular vectors etc. needed later in the local CV:
//CV ->       (V0T = transposed right singular vectors of S, Z = scaled principal components, 
//CV ->       eig = scaled, inverted singular vectors, U0 = left singular vectors of S
//CV ->       , eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
//CV ->    */ 
//CV ->    enkf_analysis_invertS_pre_cv( config , S , R , V0T , Z , eig , U0);
//CV ->    
//CV ->    
//CV ->    matrix_free( R );
//CV ->    matrix_free( S );
//CV ->    matrix_free( innov );
//CV ->        
//CV ->    if (enkf_mode == ENKF_STANDARD) {
//CV ->      matrix_free( E );
//CV ->      matrix_free( D );
//CV ->      
//CV ->    }
//CV ->  }
//CV ->  
//CV ->}
//CV ->
//CV ->
//CV ->/**
//CV ->  FUNCTION THAT COMPUTES THE PRINCIPAL COMPONENTS OF THE CENTRED DATA ENSEMBLE MATRIX:
//CV ->  
//CV ->
//CV ->  This function initializes the S matrix and performs svd(S). The
//CV ->  Function returns:
//CV ->  Z  -   The Principal Components of the empirically estimated data covariance matrix as Z (ens_size times maxP), where ens_size is the 
//CV ->             ensemble size, and maxP is the maximum number of allowed principal components
//CV ->  Rp -   (Rp = U0' * R * U0 (maxP times maxP) error covariance matrix in the reduced order subspace (Needed later in the EnKF update)
//CV ->  
//CV ->  Dp -   (Dp = U0' * D) (maxP times ens_size): Reduced data "innovation matrix".
//CV ->         where D(:,i) = dObs - dForecast(i) - Eps(i)
//CV ->*/
//CV ->
//CV ->
//CV ->void enkf_analysis_init_principal_components( double truncation , 
//CV ->                                              const matrix_type * S, 
//CV ->                                              const matrix_type * R,
//CV ->                                              const matrix_type * innov,
//CV ->                                              const matrix_type * E , 
//CV ->                                              const matrix_type * D , 
//CV ->                                              matrix_type * Z , 
//CV ->                                              matrix_type * Rp , 
//CV ->                                              matrix_type * Dp) {
//CV ->  {
//CV ->    int i, j;
//CV ->    
//CV ->    
//CV ->    const int nrobs = matrix_get_rows( S );
//CV ->    const int nrens = matrix_get_columns( S );
//CV ->    const int nrmin = util_int_min( nrobs , nrens );
//CV ->    
//CV ->    printf("Maximum number of Principal Components is %d\n",nrmin - 1);
//CV ->
//CV ->    /*
//CV ->      Compute SVD(S)
//CV ->    */
//CV ->      
//CV ->    matrix_type * U0   = matrix_alloc( nrobs , nrmin    ); /* Left singular vectors.  */
//CV ->    matrix_type * V0T  = matrix_alloc( nrmin , nrens );    /* Right singular vectors. */
//CV ->    
//CV ->    double * inv_sig0      = util_malloc( nrmin * sizeof * inv_sig0 , __func__);
//CV ->    double * sig0          = inv_sig0;
//CV ->
//CV ->    enkf_linalg_svdS(S , truncation , -1 , DGESVD_MIN_RETURN , inv_sig0 , U0 , V0T);
//CV ->    
//CV ->    /* Need to use the original non-inverted singular values. */
//CV ->    for(i = 0; i < nrmin; i++) 
//CV ->      if ( inv_sig0[i] > 0 ) 
//CV ->        sig0[i] = 1.0 / inv_sig0[i];
//CV ->    
//CV ->    
//CV ->    
//CV ->    /*
//CV ->      Compute the actual principal components, Z = sig0 * VOT 
//CV ->      NOTE: Z contains potentially alot of redundant zeros, but 
//CV ->      we do not care about this for now
//CV ->    */
//CV ->    
//CV ->    for(i = 0; i < nrmin; i++) 
//CV ->      for(j = 0; j < nrens; j++) 
//CV ->        matrix_iset( Z , i , j , sig0[i] * matrix_iget( V0T , i , j ) );
//CV ->    
//CV ->
//CV ->    /* Also compute Rp */
//CV ->    {
//CV ->      matrix_type * X0 = matrix_alloc( nrmin , matrix_get_rows( R ));
//CV ->      matrix_dgemm(X0 , U0 , R  , true  , false , 1.0 , 0.0);   /* X0 = U0^T * R */
//CV ->      matrix_dgemm(Rp  , X0 , U0 , false , false , 1.0 , 0.0);  /* Rp = X0 * U0 */
//CV ->      matrix_free( X0 );
//CV ->    }
//CV ->
//CV ->    /*We also need to compute the reduced "Innovation matrix" Dp = U0' * D    */
//CV ->    matrix_dgemm(Dp , U0 , D , true , false , 1.0 , 0.0);
//CV ->    
//CV ->        
//CV ->    free(inv_sig0);
//CV ->    matrix_free(U0);
//CV ->    matrix_free(V0T);
//CV ->    
//CV ->    /* 
//CV ->       2: Diagonalize the S matrix; singular vectors etc. needed later in the local CV:
//CV ->       (V0T = transposed right singular vectors of S, Z = scaled principal components, 
//CV ->       eig = scaled, inverted singular vectors, U0 = left singular vectors of S
//CV ->       , eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
//CV ->    */ 
//CV ->    /*   enkf_analysis_invertS_pre_cv( config , S , R , V0T , Z , eig , U0);*/
//CV ->    
//CV ->  }
//CV ->  
//CV ->  
//CV ->}
//CV ->
//CV ->
//CV ->/*Matrix that computes and returns the X5 matrix used in the EnKF updating */
//CV ->void enkf_analysis_initX_principal_components_cv( int nfolds_CV , 
//CV ->                                                  bool penalised_press , 
//CV ->                                                  matrix_type * X , 
//CV ->                                                  rng_type * rng, 
//CV ->                                                  const matrix_type * A , 
//CV ->                                                  const matrix_type * Z , 
//CV ->                                                  const matrix_type * Rp , 
//CV ->                                                  const matrix_type * Dp ) {
//CV ->  int ens_size = matrix_get_columns( Dp );
//CV ->  {
//CV ->
//CV ->    int i, j, k;
//CV ->    double tmp;
//CV ->    
//CV ->    
//CV ->    /*
//CV ->      1: Allocating all matrices
//CV ->    */
//CV ->    /*Need a copy of A, because we need it later */
//CV ->    matrix_type * workA      = matrix_alloc_copy( A );    /* <- This is a massive memory requirement. */
//CV ->    
//CV ->    /* Subtracting the ensemble mean of the state vector ensemble */
//CV ->    matrix_subtract_row_mean( workA );
//CV ->    /* 
//CV ->       2: Diagonalize the S matrix; singular vectors are stored in W
//CV ->          and singular values (after some massage) are stored in eig. 
//CV ->       W = X1, eig = inv(I+Lambda1),(Eq.14.30, and 14.29, Evensen, 2007, respectively)
//CV ->    */
//CV ->
//CV ->    
//CV ->    
//CV ->    int nrmin = matrix_get_rows( Z );
//CV ->    int maxP  = nrmin;
//CV ->
//CV ->    /* We only want to search the non-zero eigenvalues */
//CV ->    for (int i = 0; i < nrmin; i++) {
//CV ->      if (matrix_iget(Z,i,1) == 0.0) {
//CV ->        maxP = i;
//CV ->        break;
//CV ->      }
//CV ->    }
//CV ->    
//CV ->    if (maxP > nrmin) {
//CV ->      maxP = nrmin;
//CV ->    }
//CV ->
//CV ->
//CV ->
//CV ->    
//CV ->    /* Get the optimal number of principal components 
//CV ->       where p is found minimizing the PRESS statistic */
//CV ->    
//CV ->    int optP = get_optimal_principal_components(Z , Rp , nfolds_CV, workA , rng , maxP, penalised_press);
//CV ->    matrix_free( workA );
//CV ->
//CV ->    matrix_type * W          = matrix_alloc(ens_size , optP);                      
//CV ->
//CV ->    /* Compute  W = Z(1:p,:)' * inv(Z(1:p,:) * Z(1:p,:)' + (ens_size-1) * Rp(1:p,1:p))*/
//CV ->    getW_prin_comp( W , Z , Rp, optP);
//CV ->
//CV ->    /*Compute the actual X5 matrix: */
//CV ->    /*Compute X5 = W * Dp (The hard way) */
//CV ->    for( i = 0; i < ens_size; i++) {
//CV ->      for( j = 0; j < ens_size; j++) {
//CV ->        tmp = 0.0;
//CV ->        for(k = 0; k < optP; k++) {
//CV ->          tmp += matrix_iget( W , i , k) * matrix_iget( Dp , k , j);
//CV ->        }
//CV ->        
//CV ->        matrix_iset(X , i , j ,tmp);
//CV ->      }
//CV ->    }
//CV ->
//CV ->    matrix_free( W );
//CV ->    
//CV ->    /*Add one on the diagonal of X: */
//CV ->    for(i = 0; i < ens_size; i++) {
//CV ->      matrix_iadd( X , i , i , 1.0); /*X5 = I + X5 */
//CV ->    }
//CV ->    
//CV ->    enkf_analysis_checkX(X , false);
//CV ->  }
//CV ->}










