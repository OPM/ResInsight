/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rml_enkf_imodel.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/rml_enkf_common.h>
#include <ert/analysis/std_enkf.h>

/*
  A random 'magic' integer id which is used for run-time type checking
  of the input data. 
*/
#define RML_ENKF_IMODEL_TYPE_ID 261123

typedef struct rml_enkf_imodel_data_struct rml_enkf_imodel_data_type;


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
  The configuration data used by the rml_enkf_imodel module is contained in a
  rml_enkf_imodel_data_struct instance. The data type used for the rml_enkf_imodel
  module is quite simple; with only a few scalar variables, but there
  are essentially no limits to what you can pack into such a datatype.

  All the functions in the module have a void pointer as the first
  argument, this will immediately be casted to a rml_enkf_imodel_data_type
  instance, to get some type safety the UTIL_TYPE_ID system should be
  used (see documentation in util.h)

  The data structure holding the data for your analysis module should
  be created and initialized by a constructor, which should be
  registered with the '.alloc' element of the analysis table; in the
  same manner the desctruction of this data should be handled by a
  destructor or free() function registered with the .freef field of
  the analysis table.
*/




struct rml_enkf_imodel_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  double    truncation;            // Controlled by config key: ENKF_TRUNCATION_KEY
  int       subspace_dimension;    // Controlled by config key: ENKF_NCOMP_KEY (-1: use Truncation instead)
  long      option_flags;
  int       iteration_nr;          // Keep track of the outer iteration loop
  double    lamda;                 // parameter to control the search direction in Marquardt levenberg optimization 
  double    Sk;                    // Objective function value
  double    Std;                   // Standard Deviation of the Objective function
  double  * Csc;
  matrix_type * Am;
  matrix_type *prior;
  matrix_type *state;
};


/*
  This is a macro which will expand to generate a function:

     rml_enkf_imodel_data_type * rml_enkf_imodel_data_safe_cast( void * arg ) {}

  which is used for runtime type checking of all the functions which
  accept a void pointer as first argument. 
*/
static UTIL_SAFE_CAST_FUNCTION( rml_enkf_imodel_data , RML_ENKF_IMODEL_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION_CONST( rml_enkf_imodel_data , RML_ENKF_IMODEL_TYPE_ID )


double rml_enkf_imodel_get_truncation( rml_enkf_imodel_data_type * data ) {
  return data->truncation;
}

int rml_enkf_imodel_get_subspace_dimension( rml_enkf_imodel_data_type * data ) {
  return data->subspace_dimension;
}

void rml_enkf_imodel_set_truncation( rml_enkf_imodel_data_type * data , double truncation ) {
  data->truncation = truncation;
  if (truncation > 0.0)
    data->subspace_dimension = INVALID_SUBSPACE_DIMENSION;
}

void rml_enkf_imodel_set_subspace_dimension( rml_enkf_imodel_data_type * data , int subspace_dimension) {
  data->subspace_dimension = subspace_dimension;
  if (subspace_dimension > 0)
    data->truncation = INVALID_TRUNCATION;
}



void * rml_enkf_imodel_data_alloc( rng_type * rng) {
  rml_enkf_imodel_data_type * data = util_malloc( sizeof * data);
  UTIL_TYPE_ID_INIT( data , RML_ENKF_IMODEL_TYPE_ID );
  
  rml_enkf_imodel_set_truncation( data , DEFAULT_ENKF_TRUNCATION_ );
  rml_enkf_imodel_set_subspace_dimension( data , DEFAULT_SUBSPACE_DIMENSION );
  data->option_flags = ANALYSIS_NEED_ED + ANALYSIS_UPDATE_A;
  data->iteration_nr = 0;
  data->Std          = 0; 
  return data;
}


void rml_enkf_imodel_data_free( void * data ) { 
  free( data );
}


void rml_enkf_imodel_init1__( matrix_type * A,
                              rml_enkf_imodel_data_type * data, 
                              double truncation,
                              double nsc) {
  

  int nstate        = matrix_get_rows( A );
  int ens_size      = matrix_get_columns( A );
  int nrmin         = util_int_min( ens_size , nstate); 
  matrix_type * Dm  = matrix_alloc_copy( A );
  
  matrix_type * Um  = matrix_alloc( nstate , nrmin  ); /* Left singular vectors.  */
  matrix_type * VmT = matrix_alloc( nrmin , ens_size );    /* Right singular vectors. */
  double * Wm       = util_calloc( nrmin , sizeof * Wm ); 

 
   matrix_subtract_row_mean(Dm);


  //This routine only computes the SVD of Ensemble State matrix  

  
  for (int i=0; i<nstate; i++){
    double sc = nsc/ (data->Csc[i]);
    matrix_scale_row (Dm,i, sc);
  }


  int nsign1 = enkf_linalg_svd_truncation(Dm , truncation , -1 , DGESVD_MIN_RETURN  , Wm , Um , VmT);
  printf("The significant Eigen values are %d\n",nsign1);

  enkf_linalg_rml_enkfAm(Um, Wm, nsign1);
  data->Am=matrix_alloc_copy(Um);
  printf("\n Init1 completed\n");
  matrix_free(Um);
  matrix_free(VmT);
  matrix_free(Dm);
  free(Wm);
 
}

void  rml_enkf_imodel_Create_Csc(rml_enkf_imodel_data_type * data){
  // Create the scaling matrix based on the state vector

  int nstate        = matrix_get_rows( data->prior );
  int ens_size      = matrix_get_columns( data->prior );
 
  
 for (int i=0; i < nstate; i++) {
   double sumrow = matrix_get_row_sum(data->prior , i);
    double tmp = sumrow / ens_size;
    if (abs(tmp)< 1)
      data->Csc[i]=0.05;
    else
      data->Csc[i]= 1;
 }

}

void rml_enkf_imodel_scalingA(matrix_type *A, double * Csc, bool invert ){
  int nrows = matrix_get_rows(A);
  if (invert)
    for (int i=0; i< nrows ; i++)
      {
        double sc= 1/Csc[i];
        matrix_scale_row(A, i, sc);
      }
  else
    for (int i=0; i< nrows ; i++)
      {
        double sc= Csc[i];
        matrix_scale_row(A, i, sc);
      }
}



void rml_enkf_imodel_init2__( rml_enkf_imodel_data_type * data,
                              matrix_type *A,
                              matrix_type *Acopy,
                              double * Wdr,
                              double nsc,
                              matrix_type * VdTr) {


  int nstate        = matrix_get_rows( Acopy );
  int ens_size      = matrix_get_columns( Acopy );
  matrix_type * Dk  = matrix_alloc_copy( Acopy );

  double a = data->lamda + 1;
  matrix_type *Am= matrix_alloc_copy(data->Am);
  matrix_type *Apr= matrix_alloc_copy(data->prior);
  double *Csc = util_calloc(nstate , sizeof * Csc ); 
  for (int i=0; i< nstate ; i++)
    {
      Csc[i]= data->Csc[i];
    }
  int nsign1= matrix_get_columns(data->Am);
  

  matrix_type * X4  = matrix_alloc(nsign1,ens_size);
  matrix_type * X5  = matrix_alloc(nstate,ens_size);
  matrix_type * X6  = matrix_alloc(ens_size,ens_size);
  matrix_type * X7  = matrix_alloc(ens_size,ens_size);
  matrix_type * dA2 = matrix_alloc(nstate, ens_size);
  

  //Compute dA2 
  printf("\n Starting init 2 \n");
  matrix_inplace_sub(Dk, Apr);
  rml_enkf_imodel_scalingA(Dk,Csc,true);

  enkf_linalg_rml_enkfX4(X4, Am, Dk);
  matrix_free(Dk);

  enkf_linalg_rml_enkfX5(X5, Am, X4);
  printf("\nMatrix X5 computed\n");

  matrix_type * Dk1  = matrix_alloc_copy( Acopy );
  matrix_subtract_row_mean(Dk1);
  rml_enkf_imodel_scalingA(Dk1,Csc,true);
  matrix_scale(Dk1,nsc);

  enkf_linalg_rml_enkfX6(X6, Dk1,X5);
  printf("Matrix X6 computed!\n");

  enkf_linalg_rml_enkfX7(X7, VdTr ,Wdr, a, X6);
  printf("Matrix X7 computed!\n");

  rml_enkf_imodel_scalingA(Dk1,Csc,false);
  printf("Matrix Dk1 Scaling done!\n");

  enkf_linalg_rml_enkfXdA2(dA2,Dk1,X7);
  printf("Matrix dA2 computed!\n");

  matrix_inplace_sub(A, dA2);

  free(Csc);
  matrix_free(Am);
  matrix_free(Apr);
  matrix_free(X4); 
  matrix_free(X5);
  matrix_free(X6);
  matrix_free(X7);
  matrix_free(dA2);
  matrix_free(Dk1);

    
}



void rml_enkf_imodel_updateA(void * module_data , 
                      matrix_type * A , 
                      matrix_type * S , 
                      matrix_type * R , 
                      matrix_type * dObs , 
                      matrix_type * E , 
                      matrix_type * D) {


  rml_enkf_imodel_data_type * data = rml_enkf_imodel_data_safe_cast( module_data );
  double truncation = data->truncation;
  
  double Sk_new;
  double  Std_new;
  matrix_type * Skm = matrix_alloc(matrix_get_columns(D),matrix_get_columns(D));
  FILE *fp = util_fopen("rml_enkf_imodel_output","a");
  
  int nstate        = matrix_get_rows(A);
  int ens_size      = matrix_get_columns( S );
  int nrobs         = matrix_get_rows( S );
  matrix_type * Cd  = matrix_alloc( nrobs, nrobs );

  double nsc = 1/sqrt(ens_size-1); 
 

  int nrmin         = util_int_min( ens_size , nrobs); 
  matrix_type * Ud  = matrix_alloc( nrobs , nrmin    ); /* Left singular vectors.  */
  matrix_type * VdT = matrix_alloc( nrmin , ens_size );    /* Right singular vectors. */
  double * Wd       = util_calloc( nrmin , sizeof * Wd ); 
  
  enkf_linalg_Covariance(Cd ,E ,nsc, nrobs);
  matrix_inv(Cd);

  
  if (data->iteration_nr == 0) {
    Sk_new = enkf_linalg_data_mismatch(D,Cd,Skm);  //Calculate the intitial data mismatch term
    Std_new= matrix_diag_std(Skm,Sk_new);

    data->lamda =pow(10,floor(log10(Sk_new/(2*nrobs))));
    data->prior = matrix_alloc_copy(A);
    data->state = matrix_alloc_copy(A);

    data->Csc     = util_calloc(nstate , sizeof * data->Csc);
    rml_enkf_imodel_Create_Csc(data);
    rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lamda,Ud,Wd,VdT);
    printf("\n model scaling matrix computed\n");
    rml_enkf_imodel_init1__(data->prior, data, truncation, nsc);
  
    data->Sk = Sk_new;
    data->Std= Std_new;
   
    printf("Prior Objective function value is %5.3f \n", data->Sk);

    fprintf(fp,"Iteration number\t   Lamda Value \t    Current Mean (OB FN) \t    Old Mean\t     Current Stddev\n");
    fprintf(fp, "\n\n");
    fprintf(fp,"%d     \t\t       NA       \t      %5.5f      \t         \t   %5.5f    \n",data->iteration_nr, Sk_new, Std_new);
   
  }
  
  else
    {
      matrix_type *Acopy  = matrix_alloc_copy (A);
      Sk_new = enkf_linalg_data_mismatch(D,Cd,Skm);  //Calculate the intitial data mismatch term
      Std_new= matrix_diag_std(Skm,Sk_new);
      printf(" Current Objective function value is %5.3f \n\n",Sk_new);
      printf("The old Objective function value is %5.3f \n", data->Sk);


      if ((Sk_new< (data->Sk)) && (Std_new<= (data->Std)))
        {
          if ( (1- (Sk_new/data->Sk)) < .0001)  // check convergence ** model change norm has to be added in this!!
            data-> iteration_nr = 16;

          fprintf(fp,"%d     \t\t      %5.5f      \t      %5.5f      \t    %5.5f    \t   %5.5f    \n",data->iteration_nr,data->lamda, Sk_new,data->Sk, Std_new);
          data->lamda = data->lamda / 10 ;
          data->Std   = Std_new;
          data->state = matrix_alloc_copy(A);
          data->Sk = Sk_new;
          rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lamda,Ud,Wd,VdT);
          rml_enkf_imodel_init2__(data,A,Acopy,Wd,nsc,VdT);
        }
      else if((Sk_new< (data->Sk)) && (Std_new > (data->Std)))
        {
          if ( (1- (Sk_new/data->Sk)) < .0001)  // check convergence ** model change norm has to be added in this!!
            data-> iteration_nr = 16;

          fprintf(fp,"%d     \t\t      %5.5f      \t      %5.5f      \t    %5.5f    \t   %5.5f    \n",data->iteration_nr,data->lamda, Sk_new,data->Sk, Std_new);
          data->lamda = data->lamda;
          data->Std=Std_new;
          data->state = matrix_alloc_copy(A);
          data->Sk = Sk_new;
          rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lamda,Ud,Wd,VdT);
          rml_enkf_imodel_init2__(data,A,Acopy,Wd,nsc,VdT);

        }
      else {
          fprintf(fp,"%d     \t\t      %5.5f      \t      %5.5f      \t    %5.5f    \t   %5.5f     \n",data->iteration_nr,data->lamda, Sk_new,data->Sk, Std_new);
        printf("The previous step is rejected !!\n");
        data->lamda = data ->lamda * 4;   
        matrix_assign( A , data->state );
        printf("matrix copied \n");
        rml_enkf_common_initA__(A,S,Cd,E,D,truncation,data->lamda,Ud,Wd,VdT);
        rml_enkf_imodel_init2__(data,A,Acopy,Wd,nsc,VdT);
        data->iteration_nr--;
      }
      matrix_free(Acopy);
    }
  data->iteration_nr++;
  
  //setting the lower bound for lamda
  if (data->lamda <.01)
    data->lamda= .01;


  printf ("The current iteration number is %d \n ", data->iteration_nr);

  // free(data->Csc);
  matrix_free(Ud);
  matrix_free(VdT);
  free(Wd);
  matrix_free(Skm);
  matrix_free(Cd);
  fclose(fp);

}





bool rml_enkf_imodel_set_double( void * arg , const char * var_name , double value) {
  rml_enkf_imodel_data_type * module_data = rml_enkf_imodel_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , ENKF_TRUNCATION_KEY_) == 0)
      rml_enkf_imodel_set_truncation( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


bool rml_enkf_imodel_set_int( void * arg , const char * var_name , int value) {
  rml_enkf_imodel_data_type * module_data = rml_enkf_imodel_data_safe_cast( arg );
  {
    bool name_recognized = true;
    
    if (strcmp( var_name , ENKF_NCOMP_KEY_) == 0)
      rml_enkf_imodel_set_subspace_dimension( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


long rml_enkf_imodel_get_options( void * arg , long flag ) {
  rml_enkf_imodel_data_type * module_data = rml_enkf_imodel_data_safe_cast( arg );
  {
    return module_data->option_flags;
  }
}



 bool rml_enkf_imodel_has_var( const void * arg, const char * var_name) {
   const rml_enkf_imodel_data_type * module_data = rml_enkf_imodel_data_safe_cast_const( arg );
   {
     if (strcmp(var_name , "ITER") == 0)
       return true;
     else
       return false;
   }
 }



 
 int rml_enkf_imodel_get_int( const void * arg, const char * var_name) {
   const rml_enkf_imodel_data_type * module_data = rml_enkf_imodel_data_safe_cast_const( arg );
   {
     if (strcmp(var_name , "ITER") == 0)
       return module_data->iteration_nr;
     else
       return -1;
   }
 }




/**
   gcc -fpic -c <object_file> -I??  <src_file>
   gcc -shared -o <lib_file> <object_files>
*/



#ifdef INTERNAL_LINK
#define SYMBOL_TABLE rml_enkf_imodel_symbol_table
#else
#define SYMBOL_TABLE EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type SYMBOL_TABLE = {
    .alloc           = rml_enkf_imodel_data_alloc,
    .freef           = rml_enkf_imodel_data_free,
    .set_int         = rml_enkf_imodel_set_int , 
    .set_double      = rml_enkf_imodel_set_double , 
    .set_bool        = NULL , 
    .set_string      = NULL , 
    .get_options     = rml_enkf_imodel_get_options , 
    .initX           = NULL,
    .updateA         = rml_enkf_imodel_updateA ,  
    .init_update     = NULL,
    .complete_update = NULL,
    .has_var         = rml_enkf_imodel_has_var,
    .get_int         = rml_enkf_imodel_get_int,
    .get_double      = NULL,
    .get_ptr         = NULL, 
};

