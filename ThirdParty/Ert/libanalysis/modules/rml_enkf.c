/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'rml_enkf.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/rng.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/bool_vector.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/std_enkf.h>

#include <rml_enkf_common.h>
#include <rml_enkf_config.h>
#include <rml_enkf_log.h>

typedef struct rml_enkf_data_struct rml_enkf_data_type;



//**********************************************
// DEFAULT PARAMS
//**********************************************
/*
  Observe that only one of the settings subspace_dimension and
  truncation can be valid at a time; otherwise the svd routine will
  fail. This implies that the set_truncation() and
  set_subspace_dimension() routines will set one variable, AND
  INVALIDATE THE OTHER. For most situations this will be OK, but if
  you have repeated calls to both of these functions the end result
  might be a surprise.
*/



#define  USE_PRIOR_KEY               "USE_PRIOR"
#define  LAMBDA_REDUCE_FACTOR_KEY    "LAMBDA_REDUCE"
#define  LAMBDA_INCREASE_FACTOR_KEY  "LAMBDA_INCREASE"
#define  LAMBDA0_KEY                 "LAMBDA0"
#define  LAMBDA_MIN_KEY              "LAMBDA_MIN"
#define  LAMBDA_RECALCULATE_KEY      "LAMBDA_RECALCULATE"
#define  ITER_KEY                    "ITER"
#define  LOG_FILE_KEY                "LOG_FILE"
#define  CLEAR_LOG_KEY               "CLEAR_LOG"




#define RML_ENKF_TYPE_ID 261123


//**********************************************
// RML "object" data definition
//**********************************************
/*
  The configuration data used by the rml_enkf module is contained in a
  rml_enkf_data_struct instance. The data type used for the rml_enkf
  module is quite simple; with only a few scalar variables, but there
  are essentially no limits to what you can pack into such a datatype.

  All the functions in the module have a void pointer as the first
  argument, this will immediately be casted to a rml_enkf_data_type
  instance, to get some type safety the UTIL_TYPE_ID system should be
  used (see documentation in util.h)

  The data structure holding the data for your analysis module should
  be created and initialized by a constructor, which should be
  registered with the '.alloc' element of the analysis table; in the
  same manner the desctruction of this data should be handled by a
  destructor or free() function registered with the .freef field of
  the analysis table.
*/





struct rml_enkf_data_struct {
  UTIL_TYPE_ID_DECLARATION;

  int       iteration_nr;          // Keep track of the outer iteration loop
  double    Sk;                    // Objective function value
  double    Std;                   // Standard Deviation of the Objective function
  double  * Csc;
  bool_vector_type * ens_mask;

  matrix_type *Am;                 // Scaled right singular vectors of ensemble anomalies.

  matrix_type *global_prior;       // m_pr
  matrix_type *previous_state;     // m_l


  double    lambda;               // parameter to control the setp length in Marquardt levenberg optimization


  rml_enkf_log_type    * rml_log;
  rml_enkf_config_type * config;
};



static UTIL_SAFE_CAST_FUNCTION( rml_enkf_data , RML_ENKF_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION_CONST( rml_enkf_data , RML_ENKF_TYPE_ID )






//**********************************************
// Set / Get
//**********************************************


void rml_enkf_set_iteration_nr( rml_enkf_data_type * data , int iteration_nr) {
  data->iteration_nr = iteration_nr;
}

int rml_enkf_get_iteration_nr( const rml_enkf_data_type * data ) {
  return data->iteration_nr;
}





//**********************************************
// Log-file related stuff
//**********************************************


static void rml_enkf_write_log_header( rml_enkf_data_type * data, const char * format) {
  if (rml_enkf_log_is_open( data->rml_log )) {
    const char * column1 = "Iter#";
    const char * column2 = "Lambda";
    const char * column3 = "Sk old";
    const char * column4 = "Sk_new";
    const char * column5 = "std(Sk)";

    rml_enkf_log_line(data->rml_log, format, column1, column2, column3, column4, column5);
  }
}

static void rml_enkf_write_iter_info( rml_enkf_data_type * data , double prev_Sk , double Sk_new, double Std_new ) {
  if (rml_enkf_log_is_open( data->rml_log )) {

    const char * format =         "\n%2d-->%-2d %-7.3f %-7.3f --> %-7.3f %-7.3f";
    const char * format_headers = "\n%-7s %-7s %-7s --> %-7s %-7s";
    static bool has_printed_header = false;

    if (!has_printed_header) {
      rml_enkf_write_log_header( data, format_headers );
      has_printed_header = true;
    }

    rml_enkf_log_line( data->rml_log , format, data->iteration_nr, data->iteration_nr+1,  data->lambda, prev_Sk, Sk_new, Std_new);
  }
}





//**********************************************
// Memory
//**********************************************
void * rml_enkf_data_alloc( rng_type * rng) {
  rml_enkf_data_type * data = util_malloc( sizeof * data);
  UTIL_TYPE_ID_INIT( data , RML_ENKF_TYPE_ID );

  data->config       = rml_enkf_config_alloc();
  data->rml_log      = rml_enkf_log_alloc();

  data->Csc          = NULL;
  data->iteration_nr = 0;
  data->Std          = 0;
  data->previous_state = matrix_alloc(1,1);
  data->global_prior = NULL;
  data->ens_mask     = NULL;
  return data;
}

void rml_enkf_data_free( void * arg ) {
  rml_enkf_data_type * data = rml_enkf_data_safe_cast( arg );

  matrix_free( data->previous_state );
  if (data->global_prior)
    matrix_free( data->global_prior );

  rml_enkf_log_free( data->rml_log );
  rml_enkf_config_free( data->config );
  free( data );
}





//**********************************************
// Notation
//**********************************************
/*
 * X1-X7, intermediate calculations in iterations. See D.Oliver algorithm
 *
 * Variable name in code <-> D.Oliver notation       <-> Description
 * -------------------------------------------------------------------------------------------------------------
 * A                     <-> m_l                     <-> Ensemble matrix. Updated in-place by iterations.
 * data->previous_state    <-> m_(l-1)                 <-> "A" from the previous iteration. Backs up A in case the update is bad.
 * data->global_prior    <->                         <-> Previously: "active_prior". Stores A from before iter0, i.e. the actual prior.
 * Acopy                 <->                         <-> Eliminated from code. Copy of A (at each iteration, before acceptance/rejection decision)

 *
 * Am                    <-> A_m                     <-> Am = Um*Wm^(-1)
 * Csc                   <-> C_sc^(1/2)              <-> State scalings. Note the square root.
 * Dm  (in init1__)      <-> Delta m                 <-> Anomalies of prior wrt. its mean (row i scaled by 1/(Csc[i]*sqrt(N-1)))
 * Dm  (in initA__)      <-> Csc * Delta m           <-> Anomalies of A wrt. its mean (only scaled by 1/sqrt(N-1))
 * Dk1 (in init2__)      <-> Delta m                 <-> Anomailes of A (row i scaled by 1/(Csc[i]*sqrt(N-1)))
 * Dk  (in init2__)      <-> C_sc^(-1) * (m - m_pr ) <-> Anomalies wrt. prior (as opposed to the mean; only scaled by Csc)
 * dA1 (in initA__)      <-> delta m_1               <-> Ensemble updates coming from data mismatch
 * dA2 (in init2__)      <-> delta m_2               <-> Ensemble updates coming from prior mismatch
*/




//**********************************************
// Actual Algorithm, called through updateA()
//**********************************************

// Just (pre)calculates data->Am = Um*Wm^(-1).
static void rml_enkf_init1__( rml_enkf_data_type * data) {
  // Differentiate this routine from init2__, which actually calculates the prior mismatch update.
  // This routine does not change any ensemble matrix.
  // Um*Wm^(-1) are the scaled, truncated, right singular vectors of data->global_prior

  matrix_type * prior = matrix_alloc_column_compressed_copy( data->global_prior , data->ens_mask);
  int state_size      = matrix_get_rows( prior );
  int ens_size        = matrix_get_columns( prior );
  int nrmin           = util_int_min( ens_size , state_size);
  matrix_type * Dm    = matrix_alloc_copy( prior );
  matrix_type * Um    = matrix_alloc( state_size , nrmin  );     /* Left singular vectors.  */
  matrix_type * VmT   = matrix_alloc( nrmin , ens_size );        /* Right singular vectors. */
  double * Wm         = util_calloc( nrmin , sizeof * Wm );
  double nsc          = 1/sqrt(ens_size - 1);

  matrix_subtract_row_mean(Dm);
  {
    const double * Csc = data->Csc;
    for (int i=0; i < state_size; i++){
      double sc = nsc / (Csc[i]);
      matrix_scale_row( Dm , i , sc);
    }
  }

  // Um Wm VmT = Dm; nsign1 = num of non-zero singular values.
  int nsign1 = enkf_linalg_svd_truncation(Dm , rml_enkf_config_get_truncation( data->config ) , -1 , DGESVD_MIN_RETURN  , Wm , Um , VmT);

  // Am = Um*Wm^(-1). I.e. scale *columns* of Um
  enkf_linalg_rml_enkfAm(Um, Wm, nsign1);

  data->Am = matrix_alloc_copy( Um );
  matrix_free(Um);
  matrix_free(VmT);
  matrix_free(Dm);
  matrix_free(prior);
  free(Wm);
}



// Creates state scaling matrix
void rml_enkf_init_Csc(const rml_enkf_data_type * data ){
  // This seems a strange choice of scaling matrix. Review?
  matrix_type * prior = matrix_alloc_column_compressed_copy( data->global_prior , data->ens_mask );
  {
    int state_size = matrix_get_rows( prior );
    int ens_size   = matrix_get_columns( prior );

    for (int row=0; row < state_size; row++) {
      double sumrow = matrix_get_row_sum(prior , row);
      double tmp    = sumrow / ens_size;

      if (abs(tmp)< 1)
        data->Csc[row] = 0.05;
      else
        data->Csc[row] = 1.00;

    }
    matrix_free( prior );
  }
}

// Calculates update from data mismatch (delta m_1). Also provides SVD for later use.
static void rml_enkf_initA__(rml_enkf_data_type * data, matrix_type * A, matrix_type * S, matrix_type * Cd, matrix_type * E, matrix_type * D, matrix_type * Udr, double * Wdr, matrix_type * VdTr) {

  int ens_size      = matrix_get_columns( S );
  int state_size    = matrix_get_rows( A );
  double nsc        = 1/sqrt(ens_size-1);
  int nsign;

  // Perform SVD of tmp, where: tmp = diag_sqrt(Cd^(-1)) * centered(S) / sqrt(N-1) = Ud * Wd * Vd(T)
  {
    int nrobs         = matrix_get_rows( S );
    matrix_type *tmp  = matrix_alloc (nrobs, ens_size);
    matrix_subtract_row_mean( S );                        // Center S
    matrix_inplace_diag_sqrt(Cd);                         // Assumes that Cd is diag!
    matrix_matmul(tmp , Cd , S );                         //
    matrix_scale(tmp , nsc);                              //

    nsign = enkf_linalg_svd_truncation(tmp , rml_enkf_config_get_truncation( data->config ) , -1 , DGESVD_MIN_RETURN  , Wdr , Udr , VdTr);
    matrix_free( tmp );
  }

  // Calc X3
  {
    matrix_type * X3  = matrix_alloc( ens_size, ens_size );
    {
      matrix_type * X1  = matrix_alloc( nsign, ens_size);
      matrix_type * X2  = matrix_alloc( nsign, ens_size );


      // See LM-EnRML algorithm in Oliver'2013 (Comp. Geo.) for meaning
      enkf_linalg_rml_enkfX1(X1, Udr ,D ,Cd );                         // X1 = Ud(T)*Cd(-1/2)*D   -- D= -(dk-d0)
      enkf_linalg_rml_enkfX2(X2, Wdr ,X1 ,data->lambda + 1 , nsign);   // X2 = ((a*Ipd)+Wd^2)^-1  * X1
      enkf_linalg_rml_enkfX3(X3, VdTr ,Wdr,X2, nsign);                 // X3 = Vd *Wd*X2

      matrix_free(X2);
      matrix_free(X1);
    }

    // Update A
    {
      matrix_type * dA1 = matrix_alloc( state_size , ens_size);
      matrix_type * Dm  = matrix_alloc_copy( A );

      matrix_subtract_row_mean( Dm );           /* Remove the mean from the ensemble of model parameters*/
      matrix_scale(Dm, nsc);

      matrix_matmul(dA1, Dm , X3);
      matrix_inplace_add(A,dA1);                // dA

      matrix_free(Dm);
      matrix_free(dA1);
    }
    matrix_free(X3);

  }
}

// Calculate prior mismatch update (delta m_2).
void rml_enkf_init2__( rml_enkf_data_type * data, matrix_type *A, double * Wdr, matrix_type * VdTr) {
  // Distinguish from init1__ which only makes preparations, and is only called at iter=0


  int state_size   = matrix_get_rows( A );
  int ens_size     = matrix_get_columns( A );
  double nsc       = 1/sqrt(ens_size-1);

  matrix_type *Am  = matrix_alloc_copy(data->Am);
  matrix_type *Apr = matrix_alloc_column_compressed_copy(data->global_prior , data->ens_mask );

 // fprintf(stdout,"\n");
 // fprintf(stdout,"A: %d x %d\n", matrix_get_rows(A), matrix_get_columns(A));
 // fprintf(stdout,"prior : %d x %d\n", matrix_get_rows(data->global_prior), matrix_get_columns(data->global_prior));
 // fprintf(stdout,"state : %d x %d\n", matrix_get_rows(data->previous_state), matrix_get_columns(data->previous_state));
 // fprintf(stdout,"Apr : %d x %d\n", matrix_get_rows(Apr), matrix_get_columns(Apr));
 // fprintf(stdout,"Am : %d x %d\n", matrix_get_rows(Am), matrix_get_columns(Am));
 // Example:
 // A            : 27760 x 10
 // prior        : 27760 x 10
 // state        : 27760 x 50
 // prior0       : 27760 x 50
 // Apr          : 27760 x 10
 // Am           : 27760 x 1


  int nsign1 = matrix_get_columns(data->Am);


  matrix_type * X4  = matrix_alloc(nsign1,ens_size);
  matrix_type * X5  = matrix_alloc(state_size,ens_size);
  matrix_type * X6  = matrix_alloc(ens_size,ens_size);
  matrix_type * X7  = matrix_alloc(ens_size,ens_size);
  matrix_type * dA2 = matrix_alloc(state_size , ens_size);
  matrix_type * Dk1 = matrix_alloc_copy( A );

  // Dk = Csc^(-1) * (A - Aprior)
  // X4 = Am' * Dk
  {
    matrix_type * Dk = matrix_alloc_copy( A );

    matrix_inplace_sub( Dk , Apr );
    rml_enkf_common_scaleA(Dk , data->Csc , true);

    matrix_dgemm(X4 , Am , Dk , true, false, 1.0, 0.0);
    matrix_free(Dk);
  }
  // X5 = Am * X4
  matrix_matmul(X5 , Am , X4);

  // Dk1 = Csc^(-1)/sqrt(N-1) * A*(I - 1/N*ones(m,N))
  matrix_subtract_row_mean(Dk1);                  // Dk1 = Dk1 * (I - 1/N*ones(m,N))
  rml_enkf_common_scaleA(Dk1 , data->Csc , true); // Dk1 = Csc^(-1) * Dk1
  matrix_scale(Dk1,nsc);                          // Dk1 = Dk1 / sqrt(N-1)

  // X6 = Dk1' * X5
  matrix_dgemm(X6, Dk1, X5, true, false, 1.0, 0.0);

  // X7
  enkf_linalg_rml_enkfX7(X7, VdTr , Wdr , data->lambda + 1, X6);

  // delta m_2
  rml_enkf_common_scaleA(Dk1 , data->Csc , false);
  matrix_matmul(dA2 , Dk1 , X7);
  matrix_inplace_sub(A, dA2);

  matrix_free(Am);
  matrix_free(Apr);
  matrix_free(X4);
  matrix_free(X5);
  matrix_free(X6);
  matrix_free(X7);
  matrix_free(dA2);
  matrix_free(Dk1);
}

// Initialize state and prior from A. Initialize lambda0, lambda. Call initA__, init1__
static void rml_enkf_updateA_iter0(rml_enkf_data_type * data, matrix_type * A, matrix_type * S, matrix_type * R, matrix_type * dObs, matrix_type * E, matrix_type * D, matrix_type * Cd) {

  int ens_size      = matrix_get_columns( S );
  int nrobs         = matrix_get_rows( S );
  int nrmin         = util_int_min( ens_size , nrobs);
  int state_size    = matrix_get_rows( A );
  matrix_type * Skm = matrix_alloc(ens_size, ens_size);    // Mismatch
  matrix_type * Ud  = matrix_alloc( nrobs , nrmin    );    /* Left singular vectors.  */
  matrix_type * VdT = matrix_alloc( nrmin , ens_size );    /* Right singular vectors. */
  double * Wd       = util_calloc( nrmin , sizeof * Wd );

  data->Csc = util_calloc(state_size , sizeof * data->Csc);
  data->Sk  = enkf_linalg_data_mismatch(D,Cd,Skm);
  data->Std = matrix_diag_std(Skm,data->Sk);

  {
    double lambda0 = rml_enkf_config_get_lambda0( data->config );
    if (lambda0 < 0)
      data->lambda = pow(10 , floor(log10(data->Sk/(2*nrobs))) );
    else
      data->lambda = lambda0;
  }


  // state = A
  rml_enkf_common_store_state( data->previous_state  , A , data->ens_mask );

  // prior = A
  data->global_prior = matrix_alloc_copy( data->previous_state );

  // Update dependant on data mismatch
  rml_enkf_initA__(data , A, S , Cd , E , D , Ud , Wd , VdT);
  // Update dependant on prior mismatch. This should be zero (coz iter0).
  // Therefore the purpose of init1__ is just to prepare some matrices.
  if (rml_enkf_config_get_use_prior(data->config)) {
    rml_enkf_init_Csc( data );
    rml_enkf_init1__( data );
  }

  rml_enkf_write_iter_info(data, data->Sk , data->Sk, data->Std);

  matrix_free( Skm );
  matrix_free( Ud );
  matrix_free( VdT );
  free( Wd );
}


void rml_enkf_updateA(void * module_data, matrix_type * A, matrix_type * S, matrix_type * R, matrix_type * dObs, matrix_type * E, matrix_type * D, const module_info_type* module_info) {
// A : ensemble matrix
// R : (Inv?) Obs error cov.
// S : measured ensemble
// dObs: observed data
// E : perturbations for obs
// D = dObs + E - S : Innovations (wrt pert. obs)
// module_info: Information on parameters/data for internal logging



  double Sk_new;   // Mismatch
  double  Std_new; // Std dev(Mismatch)
  rml_enkf_data_type * data = rml_enkf_data_safe_cast( module_data );
  int nrobs                 = matrix_get_rows( S );           // Num obs
  int ens_size              = matrix_get_columns( S );        // N
  double nsc                = 1/sqrt(ens_size-1);             // Scale factor
  matrix_type * Cd          = matrix_alloc( nrobs, nrobs );   // Cov(E), where E = measurement perturbations?


  // Empirical error covar. R is left unused. Investigate?
  enkf_linalg_Covariance(Cd ,E ,nsc, nrobs); // Cd = SampCov(E) (including (N-1) normalization)
  matrix_inv(Cd); // In-place inversion

  rml_enkf_log_open(data->rml_log , data->iteration_nr);
  fprintf(stdout,"\nIter %d --> %d", data->iteration_nr, data->iteration_nr + 1);


  if (data->iteration_nr == 0) {
    // IF ITERATION 0
    rml_enkf_updateA_iter0(data , A , S , R , dObs , E , D , Cd);
    data->iteration_nr++;
  } else {
    // IF ITERATION 1, 2, ...
    int nrmin           = util_int_min( ens_size , nrobs);      // Min(p,N)
    matrix_type * Ud    = matrix_alloc( nrobs , nrmin    );     // Left singular vectors.  */
    matrix_type * VdT   = matrix_alloc( nrmin , ens_size );     // Right singular vectors. */
    double * Wd         = util_calloc( nrmin , sizeof * Wd );   // Singular values, vector
    matrix_type * Skm   = matrix_alloc(ens_size,ens_size);      // Mismatch
    Sk_new              = enkf_linalg_data_mismatch(D,Cd,Skm);  // Skm = D'*inv(Cd)*D; Sk_new = trace(Skm)/N
    Std_new             = matrix_diag_std(Skm,Sk_new);          // Standard deviation of mismatches.


    // Lambda = Normalized data mismatch (rounded)
    if (rml_enkf_config_get_lambda_recalculate( data->config ))
      data->lambda = pow(10 , floor(log10(Sk_new / (2*nrobs))) );

    // Accept/Reject update? Lambda calculation.
    {
      bool mismatch_reduced = false;
      bool std_reduced = false;

      if (Sk_new < data->Sk)
        mismatch_reduced = true;

      if (Std_new <= data->Std)
        std_reduced = true;

      rml_enkf_write_iter_info(data, data->Sk , Sk_new, Std_new);

      if (mismatch_reduced) {
        /*
          Stop check: if ( (1- (Sk_new/data->Sk)) < .0001)  // check convergence ** model change norm has to be added in this!!
        */

        // Reduce Lambda
        if (std_reduced)
          data->lambda = data->lambda * rml_enkf_config_get_lambda_decrease_factor( data->config );

        rml_enkf_common_store_state(data->previous_state , A , data->ens_mask );

        data->Sk = Sk_new;
        data->Std=Std_new;
        data->iteration_nr++;
      } else {
        // Increase lambda
        data->lambda = data->lambda * rml_enkf_config_get_lambda_increase_factor( data->config );
        // A = data->previous_state
        rml_enkf_common_recover_state( data->previous_state , A , data->ens_mask  );
      }
    }

    // Update dependant on data mismatch (delta m_1)
    rml_enkf_initA__(data , A , S , Cd , E , D , Ud , Wd , VdT);

    // Update dependant on prior mismatch (delta m_2)
    if (rml_enkf_config_get_use_prior(data->config)) {
      rml_enkf_init_Csc( data );
      rml_enkf_init2__(data , A , Wd , VdT);
    }

    // Free
    matrix_free(Skm);
    matrix_free( Ud );
    matrix_free( VdT );
    free( Wd );
  }

  {
    double lambda_min = rml_enkf_config_get_lambda_min( data->config );
    if (data->lambda < lambda_min)
      data->lambda = lambda_min;
  }


  rml_enkf_log_close( data->rml_log );
  matrix_free(Cd);
}



void rml_enkf_init_update(void * arg,  const bool_vector_type * ens_mask, const matrix_type * S, const matrix_type * R, const matrix_type * dObs, const matrix_type * E, const matrix_type * D ) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );

  if (module_data->ens_mask)
    bool_vector_free( module_data->ens_mask );

  module_data->ens_mask = bool_vector_alloc_copy( ens_mask );
}







//**********************************************
// Set / Get basic types
//**********************************************
bool rml_enkf_set_int( void * arg , const char * var_name , int value) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , ENKF_NCOMP_KEY_) == 0)
      rml_enkf_config_set_subspace_dimension(module_data->config , value);
    else if (strcmp( var_name , ITER_KEY) == 0)
      rml_enkf_set_iteration_nr( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}

int rml_enkf_get_int( const void * arg, const char * var_name) {
  const rml_enkf_data_type * module_data = rml_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , ITER_KEY) == 0)
      return module_data->iteration_nr;
    else
      return -1;
  }
}

bool rml_enkf_set_bool( void * arg , const char * var_name , bool value) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , USE_PRIOR_KEY) == 0)
      rml_enkf_config_set_use_prior( module_data->config , value);
    else if (strcmp( var_name , CLEAR_LOG_KEY) == 0)
      rml_enkf_log_set_clear_log( module_data->rml_log , value );
    else if (strcmp( var_name , LAMBDA_RECALCULATE_KEY) == 0)
      rml_enkf_config_set_lambda_recalculate( module_data->config , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}

bool rml_enkf_get_bool( const void * arg, const char * var_name) {
  const rml_enkf_data_type * module_data = rml_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , USE_PRIOR_KEY) == 0)
      return rml_enkf_config_get_use_prior( module_data->config );
    else if (strcmp(var_name , CLEAR_LOG_KEY) == 0)
      return rml_enkf_log_get_clear_log( module_data->rml_log );
    else if (strcmp(var_name , LAMBDA_RECALCULATE_KEY) == 0)
      return rml_enkf_config_get_lambda_recalculate( module_data->config );
    else
       return false;
  }
}

bool rml_enkf_set_double( void * arg , const char * var_name , double value) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , ENKF_TRUNCATION_KEY_) == 0)
      rml_enkf_config_set_truncation( module_data->config , value );
    else if (strcmp( var_name , LAMBDA_INCREASE_FACTOR_KEY) == 0)
      rml_enkf_config_set_lambda_increase_factor( module_data->config , value );
    else if (strcmp( var_name , LAMBDA_REDUCE_FACTOR_KEY) == 0)
      rml_enkf_config_set_lambda_decrease_factor( module_data->config , value );
    else if (strcmp( var_name , LAMBDA0_KEY) == 0)
      rml_enkf_config_set_lambda0( module_data->config , value );
    else if (strcmp( var_name , LAMBDA_MIN_KEY) == 0)
      rml_enkf_config_set_lambda_min( module_data->config , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}

double rml_enkf_get_double( const void * arg, const char * var_name) {
  const rml_enkf_data_type * module_data = rml_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , LAMBDA_REDUCE_FACTOR_KEY) == 0)
      return rml_enkf_config_get_lambda_decrease_factor(module_data->config);

    if (strcmp(var_name , LAMBDA_INCREASE_FACTOR_KEY) == 0)
      return rml_enkf_config_get_lambda_increase_factor(module_data->config);

    if (strcmp(var_name , LAMBDA0_KEY) == 0)
      return rml_enkf_config_get_lambda0(module_data->config);

    if (strcmp(var_name , LAMBDA_MIN_KEY) == 0)
      return rml_enkf_config_get_lambda_min(module_data->config);

    if (strcmp(var_name , ENKF_TRUNCATION_KEY_) == 0)
      return rml_enkf_config_get_truncation( module_data->config );

    return -1;
  }
}


bool rml_enkf_set_string( void * arg , const char * var_name , const char * value) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , LOG_FILE_KEY) == 0)
      rml_enkf_log_set_log_file( module_data->rml_log , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}

long rml_enkf_get_options( void * arg , long flag ) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    return rml_enkf_config_get_option_flags( module_data->config );
  }
}

bool rml_enkf_has_var( const void * arg, const char * var_name) {
  {
    if (strcmp(var_name , ITER_KEY) == 0)
      return true;
    else if (strcmp(var_name , USE_PRIOR_KEY) == 0)
      return true;
    else if (strcmp(var_name , LAMBDA_INCREASE_FACTOR_KEY) == 0)
      return true;
    else if (strcmp(var_name , LAMBDA_REDUCE_FACTOR_KEY) == 0)
      return true;
    else if (strcmp(var_name , LAMBDA0_KEY) == 0)
      return true;
    else if (strcmp(var_name , LAMBDA_MIN_KEY) == 0)
      return true;
    else if (strcmp(var_name , LAMBDA_RECALCULATE_KEY) == 0)
      return true;
    else if (strcmp(var_name , ENKF_TRUNCATION_KEY_) == 0)
      return true;
    else if (strcmp(var_name , LOG_FILE_KEY) == 0)
      return true;
    else if (strcmp(var_name , CLEAR_LOG_KEY) == 0)
      return true;
    else
      return false;
  }
}

void * rml_enkf_get_ptr( const void * arg , const char * var_name ) {
  const rml_enkf_data_type * module_data = rml_enkf_data_safe_cast_const( arg );
  {
    if (strcmp(var_name , LOG_FILE_KEY) == 0)
      return (void *) rml_enkf_log_get_log_file( module_data->rml_log );
    else
      return NULL;
  }
}






//**********************************************
// Symbol table
//**********************************************
#ifdef INTERNAL_LINK
#define LINK_NAME RML_ENKF
#else
#define LINK_NAME EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type LINK_NAME = {
  .name            = "RML_ENKF",
  .alloc           = rml_enkf_data_alloc,
  .freef           = rml_enkf_data_free,
  .set_int         = rml_enkf_set_int ,
  .set_double      = rml_enkf_set_double ,
  .set_bool        = rml_enkf_set_bool,
  .set_string      = rml_enkf_set_string,
  .get_options     = rml_enkf_get_options ,
  .initX           = NULL,
  .updateA         = rml_enkf_updateA ,
  .init_update     = rml_enkf_init_update ,
  .complete_update = NULL,
  .has_var         = rml_enkf_has_var,
  .get_int         = rml_enkf_get_int,
  .get_double      = rml_enkf_get_double,
  .get_bool        = rml_enkf_get_bool,
  .get_ptr         = rml_enkf_get_ptr,
};

