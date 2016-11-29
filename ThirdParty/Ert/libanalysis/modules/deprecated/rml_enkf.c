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

#include <stdlib.h> 
#include <string.h> 
#include <stdio.h>
#include <math.h>

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/bool_vector.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/std_enkf.h>

#include <rml_enkf_common.h>

/*
  A random 'magic' integer id which is used for run-time type checking
  of the input data. 
*/
#define RML_ENKF_TYPE_ID 261123




/*
  Observe that only one of the settings subspace_dimension and
  truncation can be valid at a time; otherwise the svd routine will
  fail. This implies that the set_truncation() and
  set_subspace_dimension() routines will set one variable, AND
  INVALIDATE THE OTHER. For most situations this will be OK, but if
  you have repeated calls to both of these functions the end result
  might be a surprise.  
*/
#define INVALID_SUBSPACE_DIMENSION  -1
#define INVALID_TRUNCATION          -1
#define DEFAULT_SUBSPACE_DIMENSION  INVALID_SUBSPACE_DIMENSION





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


typedef struct rml_enkf_data_struct rml_enkf_data_type;

struct rml_enkf_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  double    truncation;            // Controlled by config key: ENKF_TRUNCATION_KEY
  int       subspace_dimension;    // Controlled by config key: ENKF_NCOMP_KEY (-1: use Truncation instead)
  long      option_flags;

  int       iteration_nr;          // Keep track of the outer iteration loop
  double    lambda;                // parameter to control the search direction in Marquardt levenberg optimization
  double    lambda0;               // Initial lambda value
  double    Sk;                    // Objective function value
  double    Std;                   // Standard Deviation of the Objective function
  matrix_type *state;
  bool_vector_type * ens_mask;
};


/*
  This is a macro which will expand to generate a function:

     rml_enkf_data_type * rml_enkf_data_safe_cast( void * arg ) {}

  which is used for runtime type checking of all the functions which
  accept a void pointer as first argument. 
*/
static UTIL_SAFE_CAST_FUNCTION( rml_enkf_data , RML_ENKF_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION_CONST( rml_enkf_data , RML_ENKF_TYPE_ID )


double rml_enkf_get_truncation( rml_enkf_data_type * data ) {
  return data->truncation;
}

int rml_enkf_get_subspace_dimension( rml_enkf_data_type * data ) {
  return data->subspace_dimension;
}

void rml_enkf_set_truncation( rml_enkf_data_type * data , double truncation ) {
  data->truncation = truncation;
  if (truncation > 0.0)
    data->subspace_dimension = INVALID_SUBSPACE_DIMENSION;
}

void rml_enkf_set_lambda0(rml_enkf_data_type * data , double lambda0 ) {
  data->lambda0 = lambda0; 
}

void rml_enkf_set_subspace_dimension( rml_enkf_data_type * data , int subspace_dimension) {
  data->subspace_dimension = subspace_dimension;
  if (subspace_dimension > 0)
    data->truncation = INVALID_TRUNCATION;
}

void rml_enkf_set_iteration_number( rml_enkf_data_type *data , int iteration_number ) {
  data->iteration_nr = iteration_number;
}


void * rml_enkf_data_alloc( rng_type * rng) {
  rml_enkf_data_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , RML_ENKF_TYPE_ID );
  
  rml_enkf_set_truncation( data , DEFAULT_ENKF_TRUNCATION_ );
  rml_enkf_set_subspace_dimension( data , DEFAULT_SUBSPACE_DIMENSION );
  data->option_flags = ANALYSIS_NEED_ED + ANALYSIS_UPDATE_A + ANALYSIS_ITERABLE + ANALYSIS_SCALE_DATA;
  data->iteration_nr = 0;
  data->Std          = 0; 
  data->state        = matrix_alloc(1,1); // This will be resized under use; but we need a valid instance
  data->lambda0      = -1.0;
  data->ens_mask     = bool_vector_alloc(0,false);
  return data;
}


void rml_enkf_data_free( void * module_data ) { 
  rml_enkf_data_type * data = rml_enkf_data_safe_cast( module_data );
  matrix_free( data->state );
  bool_vector_free(data->ens_mask);
  free( data );
}





/*
  About the matrix Cd: The matrix Cd is calculated based on the content
  of the E input matrix. In the original implementation this matrix was
  only calculated in the first iteration, and then reused between subsequent
  iterations.

  Due to deactivating outliers the number of active observations can change
  from one iteration to the next, if the matrix Cd is then reused between
  iterations we will get a matrix size mismatch in the linear algebra. In the
  current implementation the Cd matrix is recalculated based on the E input
  for each iteration.
 */

void rml_enkf_updateA(void * module_data , 
                      matrix_type * A , 
                      matrix_type * S , 
                      matrix_type * R , 
                      matrix_type * dObs , 
                      matrix_type * E , 
                      matrix_type * D) {


  rml_enkf_data_type * data = rml_enkf_data_safe_cast( module_data );
  double truncation = data->truncation;
  double Sk_new;
  double  Std_new;
  
  int ens_size      = matrix_get_columns( S );
  int nrobs         = matrix_get_rows( S );
  matrix_type * Cd = matrix_alloc( nrobs , nrobs);
  double nsc        = 1/sqrt(ens_size-1); 
  matrix_type * Skm = matrix_alloc(matrix_get_columns(D),matrix_get_columns(D));
  FILE *fp = util_fopen("rml_enkf_output","a");

  int nrmin         = util_int_min( ens_size , nrobs); 
  matrix_type * Ud   = matrix_alloc( nrobs , nrmin    );    /* Left singular vectors.  */
  matrix_type * VdT  = matrix_alloc( nrmin , ens_size );    /* Right singular vectors. */
  double * Wd       = util_calloc( nrmin , sizeof * Wd  ); 
  

  Cd = matrix_alloc( nrobs, nrobs );
  enkf_linalg_Covariance(Cd ,E ,nsc, nrobs);
  matrix_inv(Cd);

  if (data->iteration_nr == 0) {
    Sk_new = enkf_linalg_data_mismatch(D,Cd,Skm);                   //Calculate the intitial data mismatch term
    Std_new = matrix_diag_std(Skm,Sk_new);
    rml_enkf_common_store_state( data->state , A , data->ens_mask ); 


    
    if (data->lambda0 < 0) 
      data->lambda = pow(10,floor(log10(Sk_new/(2*nrobs))));
    else
      data->lambda = data->lambda0; 
    
    rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lambda,Ud,Wd,VdT);
    data->Sk  = Sk_new;
    data->Std = Std_new;
    printf("Prior Objective function value is %5.3f \n", data->Sk);

    fprintf(fp,"Iteration number\t   Lamda Value \t    Current Mean (OB FN) \t    Old Mean\t     Current Stddev\n");
    fprintf(fp, "\n\n");
    fprintf(fp,"%d     \t\t       NA       \t      %5.5f      \t         \t   %5.5f    \n",data->iteration_nr, Sk_new, Std_new);
   
  } else {
    Sk_new = enkf_linalg_data_mismatch(D , Cd , Skm);  //Calculate the intitial data mismatch term
    Std_new= matrix_diag_std(Skm,Sk_new);
    printf(" Current Objective function value is %5.3f \n\n",Sk_new);
    printf("The old Objective function value is %5.3f \n", data->Sk);


    if ((Sk_new< (data->Sk)) && (Std_new< (data->Std)))
    {
      if ( (1- (Sk_new/data->Sk)) < .0001)  // check convergence ** model change norm has to be added in this!!
        data-> iteration_nr = 16;


      fprintf(fp,"%d     \t\t      %5.5f      \t      %5.5f      \t    %5.5f    \t   %5.5f    \n",data->iteration_nr,data->lambda, Sk_new,data->Sk, Std_new);
      data->lambda = data->lambda / 10 ;
      data->Std   = Std_new;

      rml_enkf_common_store_state( data->state , A , data->ens_mask ); 

      data->Sk = Sk_new;
      rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lambda,Ud,Wd,VdT);
    }
    else if((Sk_new< (data->Sk)) && (Std_new > (data->Std)))
    {
      if ( (1- (Sk_new/data->Sk)) < .0001)  // check convergence ** model change norm has to be added in this!!
        data-> iteration_nr = 16;


      fprintf(fp,"%d     \t\t      %5.5f      \t      %5.5f      \t    %5.5f    \t   %5.5f    \n",data->iteration_nr,data->lambda, Sk_new,data->Sk, Std_new);
      data->Std=Std_new;

      rml_enkf_common_store_state( data->state , A , data->ens_mask ); 

      data->Sk = Sk_new;
      rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lambda,Ud,Wd,VdT);
    }
    else {
      fprintf(fp,"%d     \t\t      %5.5f      \t      %5.5f      \t    %5.5f    \t   %5.5f    \n",data->iteration_nr,data->lambda, Sk_new,data->Sk, Std_new);
      printf("The previous step is rejected !!\n");
      data->lambda = data ->lambda * 4;

      rml_enkf_common_recover_state( data->state , A , data->ens_mask );

      rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lambda,Ud,Wd,VdT);
      data->iteration_nr--;
    }
  }
  data->iteration_nr++;

  //  setting the lower bound for lambda
  if (data->lambda <.01)
    data->lambda= .01;


  printf ("The current iteration number is %d \n ", data->iteration_nr);
  

  matrix_free(Cd);
  matrix_free(Ud);
  matrix_free(VdT);
  matrix_free(Skm);
  free(Wd);
  fclose(fp);
}


void rml_enkf_init_update(void * arg , 
                          const bool_vector_type * ens_mask , 
                          const matrix_type * S , 
                          const matrix_type * R , 
                          const matrix_type * dObs , 
                          const matrix_type * E , 
                          const matrix_type * D ) {
    rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
    bool_vector_memcpy( module_data->ens_mask , ens_mask );
}



bool rml_enkf_set_double( void * arg , const char * var_name , double value) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , ENKF_TRUNCATION_KEY_) == 0)
      rml_enkf_set_truncation( module_data , value );
    else if (strcmp( var_name , ENKF_LAMBDA0_KEY_) == 0) 
      rml_enkf_set_lambda0( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


bool rml_enkf_set_int( void * arg , const char * var_name , int value) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;
    
    if (strcmp( var_name , ENKF_NCOMP_KEY_) == 0)
      rml_enkf_set_subspace_dimension( module_data , value );
    else if(strcmp( var_name , ENKF_ITER_KEY_) == 0)
      rml_enkf_set_iteration_number( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


long rml_enkf_get_options( void * arg , long flag ) {
  rml_enkf_data_type * module_data = rml_enkf_data_safe_cast( arg );
  {
    return module_data->option_flags;
  }
}



 bool rml_enkf_has_var( const void * arg, const char * var_name) {
   bool ret = false; 
   
   if ((strcmp(var_name , ENKF_ITER_KEY_) == 0)       || 
       (strcmp(var_name , ENKF_TRUNCATION_KEY_) == 0) ||
       (strcmp(var_name , ENKF_LAMBDA0_KEY_) == 0)) {
     ret = true; 
   }
   return ret; 
 }



 
 int rml_enkf_get_int( const void * arg, const char * var_name) {
   const rml_enkf_data_type * module_data = rml_enkf_data_safe_cast_const( arg );
   {
     if (strcmp(var_name , ENKF_ITER_KEY_) == 0)
       return module_data->iteration_nr;
     else
       return -1;
   }
 }
 
  double rml_enkf_get_double( const void * arg, const char * var_name) {
   const rml_enkf_data_type * module_data = rml_enkf_data_safe_cast_const( arg );
   {
     if (strcmp(var_name , ENKF_TRUNCATION_KEY_) == 0) 
       return module_data->truncation;
     else if (strcmp(var_name , ENKF_LAMBDA0_KEY_) == 0)
      return module_data->lambda0; 
     else
       return -1.0;
   }
 }




/**
   gcc -fpic -c <object_file> -I??  <src_file>
   gcc -shared -o <lib_file> <object_files>
*/



#ifdef INTERNAL_LINK
#define SYMBOL_TABLE rml_enkf_symbol_table
#else
#define SYMBOL_TABLE EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type SYMBOL_TABLE = {
    .alloc           = rml_enkf_data_alloc,
    .freef           = rml_enkf_data_free,
    .set_int         = rml_enkf_set_int , 
    .set_double      = rml_enkf_set_double , 
    .set_bool        = NULL , 
    .set_string      = NULL , 
    .get_options     = rml_enkf_get_options , 
    .initX           = NULL,
    .updateA         = rml_enkf_updateA ,  
    .init_update     = rml_enkf_init_update ,
    .complete_update = NULL,
    .has_var         = rml_enkf_has_var,
    .get_int         = rml_enkf_get_int,
    .get_double      = rml_enkf_get_double,
    .get_ptr         = NULL, 
};

