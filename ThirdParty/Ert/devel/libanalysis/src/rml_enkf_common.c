

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

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/rml_enkf_common.h>




/* This program contains common functions to both rml_enkf & rml_enkf_imodel*/

void rml_enkf_common_initA__( matrix_type * A ,
                              matrix_type * S , 
                              matrix_type * Cd , 
                              matrix_type * E , 
                              matrix_type * D ,
                              double truncation,
                              double lamda,
                              matrix_type * Udr,
                              double * Wdr,
                              matrix_type * VdTr) {

  int nrobs         = matrix_get_rows( S );
  int ens_size      = matrix_get_columns( S );
  double a = lamda + 1;
  matrix_type *tmp  = matrix_alloc (nrobs, ens_size);
  double nsc = 1/sqrt(ens_size-1);

  
  printf("The lamda Value is %5.5f\n",lamda);
  printf("The Value of Truncation is %4.2f \n",truncation);

  matrix_subtract_row_mean( S );           /* Shift away the mean in the ensemble predictions*/
  matrix_inplace_diag_sqrt(Cd);
  matrix_dgemm(tmp, Cd, S,false, false, 1.0, 0.0);
  matrix_scale(tmp, nsc);
  
  printf("The Scaling of data matrix completed !\n ");


  // SVD(S)  = Ud * Wd * Vd(T)
  int nsign = enkf_linalg_svd_truncation(tmp , truncation , -1 , DGESVD_MIN_RETURN  , Wdr , Udr , VdTr);
  
  /* After this we only work with the reduced dimension matrices */
  
  printf("The number of siginificant ensembles are %d \n ",nsign);
  
  matrix_type * X1   = matrix_alloc( nsign, ens_size);
  matrix_type * X2    = matrix_alloc (nsign, ens_size );
  matrix_type * X3    = matrix_alloc (ens_size, ens_size );
  
  
  // Compute the matrices X1,X2,X3 and dA 
  enkf_linalg_rml_enkfX1(X1, Udr ,D ,Cd );  //X1 = Ud(T)*Cd(-1/2)*D   -- D= -(dk-d0)
  enkf_linalg_rml_enkfX2(X2, Wdr ,X1 ,a, nsign);  //X2 = ((a*Ipd)+Wd^2)^-1  * X1

  matrix_free(X1);

  enkf_linalg_rml_enkfX3(X3, VdTr ,Wdr,X2, nsign);  //X3 = Vd *Wd*X2
  printf("The X3 matrix is computed !\n ");

  matrix_type *dA1= matrix_alloc (matrix_get_rows(A), ens_size);
  matrix_type * Dm  = matrix_alloc_copy( A );

  matrix_subtract_row_mean( Dm );      /* Remove the mean from the ensemble of model parameters*/
  matrix_scale(Dm, nsc);

  enkf_linalg_rml_enkfdA(dA1, Dm, X3);      //dA = Dm * X3   
  matrix_inplace_add(A,dA1); //dA 

  matrix_free(X3);
  matrix_free(Dm);
  matrix_free(dA1);
}

