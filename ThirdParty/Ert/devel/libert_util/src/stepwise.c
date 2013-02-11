#include <stdio.h>
#include <stdlib.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/regression.h>
#include <ert/util/stepwise.h>
#include <ert/util/bool_vector.h>
#include <ert/util/double_vector.h>



#define STEPWISE_TYPE_ID 8722106

struct stepwise_struct {
  UTIL_TYPE_ID_DECLARATION;

  matrix_type      * X0;             // Externally supplied data.
  matrix_type      * Y0;
  bool               data_owner;     // Does the stepwise estimator own the data matrices X0 and Y0?

  matrix_type      * beta;           // Quantities estimated by the stepwise algorithm
  double             Y_mean; 
  matrix_type      * X_mean;
  matrix_type      * X_norm;
  bool_vector_type * active_set;
  rng_type         * rng;           // Needed in the cross-validation 
};








static double stepwise_estimate__( stepwise_type * stepwise , bool_vector_type * active_rows) {
  matrix_type * X;
  matrix_type * Y;

  double y_mean    = 0;
  int nvar         = matrix_get_columns( stepwise->X0 );
  int nsample      = matrix_get_rows( stepwise->X0 );

  nsample = bool_vector_count_equal( active_rows , true );
  nvar = bool_vector_count_equal( stepwise->active_set , true );

  
  matrix_set( stepwise->beta , 0 ); // It is essential to make sure that old finite values in the beta0 vector do not hang around.


  /*
    Extracting the data used for regression, and storing them in the
    temporary local matrices X and Y. Selecting data is based both on
    which varibles are active (stepwise->active_set) and which rows
    should be used for regression, versus which should be used for
    validation (@active_rows).
  */
  if ((nsample < matrix_get_rows( stepwise->X0 )) || (nvar < matrix_get_columns( stepwise->X0 ))) {
    X = matrix_alloc( nsample , nvar );
    Y = matrix_alloc( nsample , 1);
    
    {
      int icol,irow;   // Running over all values.
      int arow,acol;   // Running over active values.
      arow = 0;
      for (irow = 0; irow < matrix_get_rows( stepwise->X0 ); irow++) {
        if (bool_vector_iget( active_rows , irow )) {
          acol = 0;
          for (icol = 0; icol < matrix_get_columns( stepwise->X0 ); icol++) {
            if (bool_vector_iget( stepwise->active_set , icol )) {
              matrix_iset( X , arow , acol , matrix_iget( stepwise->X0 , irow , icol ));
              acol++;
            }
          }
          
          matrix_iset( Y , arow , 0 , matrix_iget( stepwise->Y0 , irow , 0 ));
          arow++;
        }
      }
    }
  } else {
    X = matrix_alloc_copy( stepwise->X0 );
    Y = matrix_alloc_copy( stepwise->Y0 );
  }
  
  
  {

    if (stepwise->X_mean != NULL)
      matrix_free( stepwise->X_mean);
    
    stepwise->X_mean = matrix_alloc( 1 , nvar );

    if (stepwise->X_norm != NULL)
      matrix_free( stepwise->X_norm);
    
    stepwise->X_norm = matrix_alloc( 1 , nvar );

    
    

    

    matrix_type * beta     = matrix_alloc( nvar , 1);           /* This is the beta vector as estimated from the OLS estimator. */
    matrix_type * tmp_beta = matrix_alloc_copy( beta );         /* This is the beta vector shifted and scaled back to original variables. */ 
    


    stepwise->Y_mean = regression_scale( X , Y , stepwise->X_mean , stepwise->X_norm );
    
    regression_OLS( X , Y , beta );
    y_mean = regression_unscale( beta , stepwise->X_norm , stepwise->X_mean , stepwise->Y_mean , tmp_beta );

    
    /*Compute the actual mean of y:
    double tmpMean = 0.0;
    for (int ii = 0; ii < nsample; ii++) {
      tmpMean += matrix_iget( Y , ii , 0 );
    }
    
    tmpMean = tmpMean / nsample;
    printf("Average of y = %0.2f\n",tmpMean); */
    
    
    

    /* 
       In this code block the beta/tmp_beta vector which is dense with
       fewer elements than the full model is scattered into the beta0
       vector which has full size and @nvar elements.
    */
    {
      int ivar,avar;   
      avar = 0;
      for (ivar = 0; ivar < matrix_get_columns( stepwise->X0 ); ivar++) {
        if (bool_vector_iget( stepwise->active_set , ivar )) {
          matrix_iset( stepwise->beta , ivar , 0 , matrix_iget( tmp_beta , avar , 0));
          avar++;
        }
      }
    }
    

    matrix_free( beta );
    matrix_free( tmp_beta );
  }
  
  matrix_free( X );
  matrix_free( Y );
  return y_mean;
}




static double stepwise_eval__( const stepwise_type * stepwise , const matrix_type * x ) { 
  return matrix_row_column_dot_product( x , 0 , stepwise->beta , 0 );
}



static double stepwise_test_var( stepwise_type * stepwise , int test_var , int blocks) {
  double prediction_error = 0;
  
  bool_vector_iset( stepwise->active_set , test_var , true );   // Temporarily activate this variable
  {

    int nvar                       = matrix_get_columns( stepwise->X0 );
    int nsample                    = matrix_get_rows( stepwise->X0 );
    int block_size                 = nsample / blocks;
    matrix_type * beta             = matrix_alloc( nvar , 1 );
    bool_vector_type * active_rows = bool_vector_alloc( nsample, true );

    

      

    /*True Cross-Validation: */
    int * randperms     = util_calloc( nsample , sizeof * randperms );
    for (int i=0; i < nsample; i++)
      randperms[i] = i;
    
    /* Randomly perturb ensemble indices */
    rng_shuffle_int( stepwise->rng , randperms , nsample );
    

    for (int iblock = 0; iblock < blocks; iblock++) {

      int validation_start = iblock * block_size;
      int validation_end   = validation_start + block_size - 1;
      
      if (iblock == (blocks - 1))
        validation_end = nsample - 1;

      /*
        Ensure that the active_rows vector has a block consisting of
        the interval [validation_start : validation_end] which is set to
        false, and the remaining part of the vector is set to true.
      */
      {
        bool_vector_reset( active_rows );

        bool_vector_iset( active_rows , nsample - 1 , true );   
        /* 
           If blocks == 1 that means all datapoint are used in the
           regression, and then subsequently reused in the R2
           calculation. 
        */
        if (blocks > 1) {                                      
          for (int i = validation_start; i <= validation_end; i++) {
            bool_vector_iset( active_rows , randperms[i] , false );
          }
        }
      }
      
  
      /*
        Evaluate the prediction error on the validation part of the
        dataset.
      */
      {
        stepwise_estimate__( stepwise , active_rows );
        {
          int irow;
          matrix_type * x_vector = matrix_alloc( 1 , nvar );
          for (irow=validation_start; irow <= validation_end; irow++) {
            matrix_copy_row( x_vector , stepwise->X0 , 0 , irow);
            
            {
              double true_value      = matrix_iget( stepwise->Y0 , irow , 0 );
              double estimated_value = stepwise_eval__( stepwise , x_vector );
              prediction_error += (true_value - estimated_value) * (true_value - estimated_value);
            }
            
          }
          matrix_free( x_vector );
        }
      }
    }
    
    matrix_free( beta );  
    free( randperms );
    bool_vector_free( active_rows );
  }
  
  /*inactivate the test_var-variable after completion*/
  bool_vector_iset( stepwise->active_set , test_var , false );  
  return prediction_error;
}






void stepwise_estimate( stepwise_type * stepwise , double deltaR2_limit , int CV_blocks) {
  int nvar          = matrix_get_columns( stepwise->X0 );
  int nsample       = matrix_get_rows( stepwise->X0 );
  double currentR2 = -1;
  bool_vector_type * active_rows = bool_vector_alloc( nsample , true );


  /*Reset beta*/
  for (int i = 0; i < nvar; i++) {
    matrix_iset(stepwise->beta, i , 0 , 0.0);
  }
  
  

  bool_vector_set_default( stepwise->active_set , false );
  bool_vector_reset( stepwise->active_set );
  bool_vector_iset( stepwise->active_set , nvar - 1 , false );

  double MSE_min = 10000000;
  double Prev_MSE_min = MSE_min;
  double minR2    = -1;  

  while (true) {
    int    best_var = 0;
    Prev_MSE_min = MSE_min;
    
    /*
      Go through all the inactive variables, and calculate the
      resulting prediction error IF this particular variable is added;
      keep track of the variable which gives the lowest prediction error.
    */
    for (int ivar = 0; ivar < nvar; ivar++) {
      if (!bool_vector_iget( stepwise->active_set , ivar)) {
        double newR2 = stepwise_test_var(stepwise , ivar , CV_blocks);
        if ((minR2 < 0) || (newR2 < minR2)) {
          minR2 = newR2;
          best_var = ivar;
        }
      }
    }


    
    printf("Current best variable  = %d\n",best_var);

    /*
      If the best relative improvement in prediction error is better
      than @deltaR2_limit, the corresponding variable is added to the
      active set, and we return to repeat the loop one more
      time. Otherwise we just exit.
    */
    
    {
      MSE_min = minR2;
      printf("MSE_min = %f\n",MSE_min);
      double deltaR2 = MSE_min / Prev_MSE_min; 
      
      printf("deltaR2 for variable %d = %0.2f\n",best_var,deltaR2);

      if (( currentR2 < 0) || deltaR2 < deltaR2_limit) {
        printf("adding %d to active set:\n",best_var);
        bool_vector_iset( stepwise->active_set , best_var , true );
        currentR2 = minR2;
        stepwise_estimate__( stepwise , active_rows );
      } else {
        printf("Done with forward stepwise regression! \n");
        /* The gain in prediction error is so small that we just leave the building. */
        /* NB! Need one final compuation of beta (since the test_var function does not reset the last tested beta value !) */
        stepwise_estimate__( stepwise , active_rows );
        break;
      } 
      
      if (bool_vector_count_equal( stepwise->active_set , true) == matrix_get_columns( stepwise->X0 )) {
        printf("All variable are active -  done with the forward stepwise regression!\n");
        stepwise_estimate__( stepwise , active_rows );
        break;   /* All variables are active. */
      }
    }

  }

  printf("Beta = ");
  for (int i = 0; i < nvar; i++) {
    printf(" %f ",matrix_iget(stepwise->beta,i,0));
  }


  
  printf("\n");



  bool_vector_free( active_rows );
}



double stepwise_eval( const stepwise_type * stepwise , const matrix_type * x ) { 
  double yHat = stepwise_eval__(stepwise, x );
  return yHat;
}



static stepwise_type * stepwise_alloc__( int nsample , int nvar , rng_type * rng) {
  stepwise_type * stepwise = util_malloc( sizeof * stepwise );
  
  stepwise->X_mean      = NULL;
  stepwise->X_norm      = NULL;
  stepwise->Y_mean      = 0.0;
  stepwise->rng         = rng;
  stepwise->X0          = NULL;
  stepwise->Y0          = NULL;
  stepwise->active_set  = bool_vector_alloc( nvar , true );
  stepwise->beta        = matrix_alloc( nvar , 1 );
  
  return stepwise;
}


stepwise_type * stepwise_alloc0( rng_type * rng) {
  stepwise_type * stepwise = util_malloc( sizeof * stepwise );
  
  stepwise->rng         = rng;
  stepwise->X0          = NULL;
  stepwise->Y0          = NULL;
  stepwise->beta        = NULL;
  stepwise->active_set  = NULL;
  stepwise->data_owner  = true;
  stepwise->X_mean      = NULL;
  stepwise->X_norm      = NULL;
  stepwise->Y_mean      = 0.0;
  
  
  
  return stepwise;
}



stepwise_type * stepwise_alloc1( int nsample , int nvar, rng_type * rng) {
  stepwise_type * stepwise = stepwise_alloc__( nsample , nvar , rng);

  stepwise->rng         = rng;
  stepwise->X0          = matrix_alloc( nsample , nvar );
  stepwise->Y0          = matrix_alloc( nsample , 1 );
  stepwise->data_owner  = true;
  
  return stepwise;
}


stepwise_type * stepwise_alloc2( matrix_type * X , matrix_type * Y , bool internal_copy , rng_type * rng) {
  stepwise_type * stepwise = stepwise_alloc__( matrix_get_rows( X ) , matrix_get_columns( X ) , rng);
  if (internal_copy) {
    stepwise->X0 = matrix_alloc_copy( X );
    stepwise->Y0 = matrix_alloc_copy( Y );
    stepwise->data_owner = true;
  } else {
    stepwise->X0 = X; 
    stepwise->Y0 = Y;
    stepwise->data_owner = false;
  }
  return stepwise;
}


void stepwise_set_Y0( stepwise_type * stepwise , matrix_type * Y) {
  if (stepwise->Y0 != NULL) {
    matrix_free( stepwise->Y0 );
  }
  
  stepwise->Y0 = Y;
}

void stepwise_set_X0( stepwise_type * stepwise ,  matrix_type * X) {
  if (stepwise->X0 != NULL)
    matrix_free( stepwise->X0 );
  

  stepwise->X0 = X;
}


void stepwise_set_beta( stepwise_type * stepwise ,  matrix_type * b) {
if (stepwise->beta != NULL)
    matrix_free( stepwise->beta );
  
  stepwise->beta = b;
}

void stepwise_set_active_set( stepwise_type * stepwise ,  bool_vector_type * a) {
  if (stepwise->active_set != NULL)
    bool_vector_free( stepwise->active_set );
  
  stepwise->active_set = a;
}

matrix_type * stepwise_get_X0( stepwise_type * stepwise ) {
  return stepwise->X0;
}

matrix_type * stepwise_get_Y0( stepwise_type * stepwise ) {
  return stepwise->Y0;
}


int stepwise_get_nsample( stepwise_type * stepwise ) {
  return matrix_get_rows( stepwise->X0 );
}

int stepwise_get_nvar( stepwise_type * stepwise ) {
  return matrix_get_columns( stepwise->X0 );
}

void stepwise_isetY0( stepwise_type * stepwise , int i , double value ) {
  matrix_iset( stepwise->Y0, i , 0 , value );
}


void stepwise_free( stepwise_type * stepwise ) {
  if (stepwise->active_set != NULL) {
    bool_vector_free( stepwise->active_set );
  }
  

  if (stepwise->beta != NULL)
    matrix_free( stepwise->beta );



  if (stepwise->X_mean != NULL)  
    matrix_free( stepwise->X_mean );


  if (stepwise->X_norm != NULL)
    matrix_free( stepwise->X_norm );
  


  if (stepwise->data_owner) {
    matrix_free( stepwise->X0 );
    matrix_free( stepwise->Y0 );
  }
  free( stepwise );
  
}
