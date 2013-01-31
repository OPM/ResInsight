/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lars.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/int_vector.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/matrix_lapack.h>
#include <ert/util/regression.h>
#include <ert/util/lars.h>

#define LARS_TYPE_ID 77125439

struct lars_struct {
  UTIL_TYPE_ID_DECLARATION;
  matrix_type * X;
  matrix_type * Y;
  bool          data_owner;  
  double        Y0;
  matrix_type * beta0;

  matrix_type * beta;
  matrix_type * X_norm;
  matrix_type * X_mean;
  double        Y_mean; 
};



static lars_type * lars_alloc__() {
  lars_type * lars = util_malloc( sizeof * lars );
  UTIL_TYPE_ID_INIT( lars , LARS_TYPE_ID );
  lars->X = NULL;
  lars->Y = NULL;

  lars->X_norm = NULL;
  lars->X_mean = NULL;
  lars->beta   = NULL;
  lars->beta0  = NULL;
  lars->Y      = 0;
  
  return lars;
}

lars_type * lars_alloc1( int nsample , int nvars) {
  lars_type * lars = lars_alloc__();
  lars->X = matrix_alloc( nsample , nvars );
  lars->Y = matrix_alloc( nsample , 1 );
  lars->data_owner = true;
  return lars;
}


lars_type * lars_alloc2( matrix_type * X , matrix_type * Y , bool internal_copy ) {
  lars_type * lars = lars_alloc__();
  if (internal_copy) {
    lars->X = matrix_alloc_copy( X );
    lars->Y = matrix_alloc_copy( Y );
    lars->data_owner = true;
  } else {
    lars->X = X; 
    lars->Y = Y;
    lars->data_owner = false;
  }
  return lars;
}

int lars_get_sample( const lars_type * lars ) {
  return matrix_get_rows( lars->X );
}


int lars_get_nvar( const lars_type * lars ) {
  return matrix_get_columns( lars->X );
}


void lars_isetX( lars_type * lars, int sample, int var , double value) {
  matrix_iset( lars->X , sample , var , value );
}

void lars_isetY( lars_type * lars, int sample, double value) {
  matrix_iset( lars->Y , sample , 0 , value );
}

#define MATRIX_SAFE_FREE( m ) if (m != NULL) matrix_free( m )
void lars_free( lars_type * lars ) {

  if (lars->data_owner) {
    MATRIX_SAFE_FREE( lars->X );
    MATRIX_SAFE_FREE( lars->Y );
  }
  MATRIX_SAFE_FREE( lars->X_norm );
  MATRIX_SAFE_FREE( lars->X_mean );
  MATRIX_SAFE_FREE( lars->beta );
  MATRIX_SAFE_FREE( lars->beta0 );

  free( lars );
}
#undef MATRIX_SAFE_FREE

/*****************************************************************/

static double sgn(double x) {
  return copysign(1 , x); // C99
}


static void lars_estimate_init( lars_type * lars, matrix_type * X , matrix_type * Y) {
  int nvar     = matrix_get_columns( lars->X );
  
  matrix_assign( X , lars->X );
  matrix_assign( Y , lars->Y );
  if (lars->X_norm != NULL)
    matrix_free( lars->X_norm );
  lars->X_norm = matrix_alloc(1 , nvar );

  if (lars->X_mean != NULL)
    matrix_free( lars->X_mean );
  lars->X_mean = matrix_alloc(1 , nvar );

  if (lars->beta != NULL)
    matrix_free( lars->beta );
  lars->beta = matrix_alloc( nvar , nvar );
  lars->Y_mean = regression_scale( X , Y , lars->X_mean , lars->X_norm);
}


double lars_getY0( const lars_type * lars) {
  return lars->Y0;
}

double lars_iget_beta( const lars_type * lars , int index) {
  return matrix_iget( lars->beta0 , index , 0 );
}



void lars_select_beta( lars_type * lars , int beta_index) {
  int nvars = matrix_get_rows( lars->beta );
  if (lars->beta0 == NULL)
    lars->beta0 = matrix_alloc( nvars , 1 );
  {
    matrix_type * beta_vector = matrix_alloc( nvars , 1 );
    matrix_copy_column( beta_vector , lars->beta , 0 , beta_index );
    lars->Y0 = regression_unscale( beta_vector , lars->X_norm , lars->X_mean , lars->Y_mean , lars->beta0 );
    matrix_free( beta_vector );
  }
}



/*
  The algorithm can very briefly be summarized as:

  1. Determine the covariate with greatest correlation to the data and
     add this covariate to the model. In the current implementation the
     correlations are stored in the matrix @C and the greatest
     correlation is stored in the scalar variable @maxC.

  2. Determine an update direction in the set of active covariates
     which is equiangular to all covariates.
     
  3. Determine the step length @gamma - which is the exact step length
     before a new covariate will enter the active set at the current
     correlation level.

  4. Update the beta estimate and the current 'location' mu.
*/

void lars_estimate(lars_type * lars , int max_vars , double max_beta , bool verbose) {
  int nvars       = matrix_get_columns( lars->X );
  int nsample     = matrix_get_rows( lars->X );
  matrix_type * X = matrix_alloc( nsample, nvars );    // Allocate local X and Y variables
  matrix_type * Y = matrix_alloc( nsample, 1 );        // which will hold the normalized data 
  lars_estimate_init( lars , X , Y);                   // during the estimation process.
  {
    matrix_type * G                = matrix_alloc_gram( X , true );
    matrix_type * mu               = matrix_alloc( nsample , 1 );
    matrix_type * C                = matrix_alloc( nvars , 1 );
    matrix_type * Y_mu             = matrix_alloc_copy( Y ); 
    int_vector_type * active_set   = int_vector_alloc(0,0);
    int_vector_type * inactive_set = int_vector_alloc(0,0);
    int    active_size;

    
    if ((max_vars <= 0) || (max_vars > nvars))
      max_vars = nvars;
    
    {
      int i;
      for (i=0; i < nvars; i++)
        int_vector_iset( inactive_set , i , i );
    }
    matrix_set( mu , 0 );

    while (true) {
      double maxC = 0;

      /*
        The first step is to calculate the correlations between the
        covariates, and the current residual. All the currently inactive
        covariates are searched; the covariate with the greatest
        correlation with (Y - mu) is selected and added to the active set.
      */
      matrix_sub( Y_mu , Y , mu );                            // Y_mu = Y - mu 
      matrix_dgemm( C , X , Y_mu , true , false , 1.0 , 0);   // C    = X' * Y_mu
      { 
        int i;
        int max_set_index = 0;

        for (i=0; i < int_vector_size( inactive_set ); i++) {
          int    set_index = i;
          int    var_index = int_vector_iget( inactive_set , set_index );
          double value     = fabs( matrix_iget(C ,  var_index , 0) );
          if (value > maxC) {
            maxC          = value;
            max_set_index = set_index;
          }
        }
        /* 
           Remove element corresponding to max_set_index from the
           inactive set and add it to the active set:
        */
        int_vector_append( active_set , int_vector_idel( inactive_set , max_set_index ));
      }
      active_size = int_vector_size( active_set );
      /*
        Now we have calculated the correlations between all the
        covariates and the current residual @Y_mu. The correlations are
        stored in the matrix @C. The value of the maximum correlation is
        stored in @maxC.
      
        Based on the value of @maxC we have added one new covariate to
        the model, technically by moving the index from @inactive_set to
        @active_set.
      */

      /*****************************************************************/


      {
        matrix_type * weights     = matrix_alloc( active_size , 1);
        double scale;

        /*****************************************************************/
        /* This scope should compute and initialize the variables
           @weights and @scale. */
        {
          matrix_type * subG        = matrix_alloc( active_size , active_size );
          matrix_type * STS         = matrix_alloc( active_size , active_size );
          matrix_type * sign_vector = matrix_alloc( active_size , 1);
          int i , j;

          /*
            STS = S' o S where 'o' is the Schur product and S is given
            by:

            [  s1   s2   s3   s4 ]  
        S = [  s1   s2   s3   s4 ]
            [  s1   s2   s3   s4 ]
            [  s1   s2   s3   s4 ]

            Where si is the sign of the correlation between (active)
            variable 'i' and Y_mu.
          */

                
          for (i=0; i < active_size ; i++) {
            int     vari  = int_vector_iget( active_set , i );
            double  signi = sgn( matrix_iget( C , vari , 0));
            matrix_iset( sign_vector , i , 0 , signi );
            for (j=0; j < active_size; j++) {
              int     varj  = int_vector_iget( active_set , j );
              double  signj = sgn( matrix_iget( C , varj , 0));
            
              matrix_iset( STS , i , j , signi * signj );
            }
          }
        
          // Extract the elements from G corresponding to active indices and
          // copy to the matrix subG:
          for (i=0; i < active_size ; i++) {
            int ii = int_vector_iget( active_set , i );
            for (j=0; j < active_size; j++) {
              int jj = int_vector_iget( active_set , j );
            
              matrix_iset( subG , i , j , matrix_iget(G , ii , jj));
            }
          }
      
          // Weights 
          matrix_inplace_mul( subG , STS );  
          matrix_inv( subG );
        
          {
            matrix_type * ones = matrix_alloc( active_size , 1 );
            matrix_type * GA1  = matrix_alloc( active_size , 1 );
          
            matrix_set( ones , 1.0 );
            matrix_matmul( GA1 , subG , ones );
            scale = 1.0 / sqrt( matrix_get_column_sum( GA1 , 0 ));
          
            matrix_mul( weights , GA1 , sign_vector );
            matrix_scale( weights , scale );
          
            matrix_free( GA1 );
            matrix_free( ones );
          }
        
          matrix_free( sign_vector );
          matrix_free( subG );
          matrix_free( STS );
        }
      
        /******************************************************************/
        /* The variables weight and scale have been calculated, proceed
           to calculate the step length @gamma. */ 
        {
          int i;
          double  gamma;
        
          {
            matrix_type * u = matrix_alloc( nsample , 1 );
            int j;

            for (i=0; i < nsample; i++) {
              double row_sum = 0;
              for (j =0; j < active_size; j++) 
                row_sum += matrix_iget( X , i , int_vector_iget( active_set , j)) * matrix_iget(weights , j , 0 );
            
              matrix_iset( u , i , 0 , row_sum );
            }
          
            gamma = maxC / scale;
            if (active_size < matrix_get_columns( X )) {
              matrix_type * equi_corr = matrix_alloc( nvars , 1 );
              matrix_dgemm( equi_corr , X , u , true , false , 1.0 , 0);     // equi_corr = X'·u
              for (i=0; i < (nvars - active_size); i++) {
                int var_index  = int_vector_iget( inactive_set , i );
                double gamma1  = (maxC - matrix_iget(C , var_index , 0 )) / ( scale - matrix_iget( equi_corr , var_index , 0));
                double gamma2  = (maxC + matrix_iget(C , var_index , 0 )) / ( scale + matrix_iget( equi_corr , var_index , 0));
              
                if ((gamma1 > 0) && (gamma1 < gamma))
                  gamma = gamma1;
              
                if ((gamma2 > 0) && (gamma2 < gamma))
                  gamma = gamma2;
              
              }
              matrix_free( equi_corr );
            }
            /* Update the current estimated 'location' mu. */
            matrix_scale( u , gamma );
            matrix_inplace_add( mu , u );
            matrix_free( u );
          } 
      
          /* 
             We have calculated the step length @gamma, and the @weights. Update the @beta matrix.
          */
          for (i=0; i < active_size; i++) 
            matrix_iset( lars->beta , int_vector_iget( active_set , i ) , active_size - 1 , gamma * matrix_iget( weights , i , 0));
      
          if (active_size > 1) 
            for (i=0; i < nvars; i++)
              matrix_iadd( lars->beta , i , active_size - 1 , matrix_iget( lars->beta , i , active_size - 2)); 
        
          matrix_free( weights );
        }
      }
    
      if (active_size == max_vars)
        break;
      
      if (max_beta > 0) {
        double beta_norm2 = matrix_get_column_abssum( lars->beta , active_size - 1 );
        if (beta_norm2 > max_beta) {
          // We stop - we will use an interpolation between this beta estimate and
          // the previous, to ensure that the |beta| = max_beta criteria is satisfied.
          if (active_size >= 2) {
            double beta_norm1 = matrix_get_column_abssum( lars->beta , active_size - 2 );
            double s = (max_beta - beta_norm1)/(beta_norm2 - beta_norm1);
            {
              int j;
              for (j=0; j < nvars; j++) {
                double beta1 = matrix_iget( lars->beta , j , active_size - 2 );
                double beta2 = matrix_iget( lars->beta , j , active_size - 1 );
                matrix_iset( lars->beta , j , active_size - 1 , beta1 + s*(beta2 - beta1));
              }
            }
          }
          break;
        }
      }
    }
    matrix_free( G );
    matrix_free( mu );
    matrix_free( C );
    matrix_free( Y_mu );
    int_vector_free( active_set );
    int_vector_free( inactive_set );
    matrix_resize( lars->beta , nvars , active_size , true );
    if (verbose) 
      matrix_pretty_fprint( lars->beta , "beta" , "%12.5f" , stdout );
    lars_select_beta( lars , active_size - 1);
  }
  matrix_free( X );
  matrix_free( Y );
}


double lars_eval1( const lars_type * lars , const matrix_type * x) {
  return lars->Y0 + matrix_row_column_dot_product( x , 0 , lars->beta0 , 0 );
}


double lars_eval2( const lars_type * lars , double * x) {
  matrix_type * x_view = matrix_alloc_view( x , 1 , matrix_get_columns( lars->X_mean ));
  double y = lars_eval1( lars , x_view );
  matrix_free( x_view );
  return y;
}





