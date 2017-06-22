/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'matrix_lapack.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_lapack.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
    The external lapack routines
*/
/*****************************************************************/
void  dgesv_(int * n, int * nrhs , double * A , int * lda , long int * ipivot , double * B , int * ldb , int * info);
void  dgesvd_(char * jobu , char * jobvt , int * m , int * n , double * A, int * lda , double * S , double * U , int * ldu , double * VT , int * ldvt, double * work , int * worksize , int * info);
void  dsyevx_(char * jobz, char * range , char * uplo , int *n , double * A , int * lda , double * vl , double * vu , int * il , int * iu , double * abstol , int * m , double * w , double *z , int * ldz , double * work, int * lwork , int * iwork , int * ifail , int * info);
void  dgeqrf_(int * m , int * n , double * A , int * lda , double * tau , double * work , int * lwork, int * info);
void  dorgqr_(int * m, int * n , int * k , double * A , int * lda , double * tau , double * work , int * lwork, int * info);
void  dgetrf_(int * M , int * n , double * A , int * lda , int * ipiv, int * info);
void  dgedi_(double * A , int * lda , int * n , int * ipiv , double * det , double * work , int * job);
void  dgetri_( int * n , double * A , int * lda , int * ipiv , double * work , int * work_size , int * info);
/*****************************************************************/






/**
   This file implements (very thin) interfaces for the matrix_type to
   some lapack functions. The file does not contain any data
   structures, only functions.
*/



static void matrix_lapack_assert_fortran_layout( const matrix_type * matrix ) {
  int rows, columns, row_stride , column_stride;
  matrix_get_dims( matrix , &rows , &columns , &row_stride , &column_stride);
  if (!( (column_stride >= rows) && (row_stride == 1)))
    util_abort("%s: lapack routines require Fortran layout of memory - aborting \n",__func__);
}


static void matrix_lapack_assert_square(const matrix_type * matrix) {
  matrix_lapack_assert_fortran_layout(matrix );
  {
    int rows, columns, row_stride , column_stride;
    matrix_get_dims( matrix , &rows , &columns , &row_stride , &column_stride);
    if (rows != columns)
      util_abort("%s: must have square matrices \n",__func__);
  }
}


/*****************************************************************/
/**
   Solves the linear equations Ax = B. The solution is stored in B on
   return.
*/


void matrix_dgesv(matrix_type * A , matrix_type * B) {
  matrix_lapack_assert_square( A );
  matrix_lapack_assert_fortran_layout( B );
  {
    int n    = matrix_get_rows( A );
    int lda  = matrix_get_column_stride( A );
    int ldb  = matrix_get_column_stride( B );
    int nrhs = matrix_get_columns( B );
    long int * ipivot = util_calloc( n , sizeof * ipivot );
    int info;

    dgesv_(&n , &nrhs , matrix_get_data( A ) , &lda , ipivot , matrix_get_data( B ), &ldb , &info);
    if (info != 0)
      util_abort("%s: low level lapack routine: dgesv() failed with info:%d \n",__func__ , info);
    free(ipivot);
  }
}


/*****************************************************************/
/**
   Singular Value Decomposition
*/


/**
   This little function translates between an integer identifier
   (i.e. and enum instance) to one of the characters used by the low
   level lapack routine to indicate how the singular vectors should be
   returned to the calling scope.

   The meaning of the different enum values is documented in the enum
   definition in the header file matrix_lapack.h.
*/

static char dgesvd_get_vector_job( dgesvd_vector_enum vector_job) {
  char job = 'X';
  switch (vector_job) {
  case(DGESVD_ALL):
    job = 'A';
    break;
  case(DGESVD_MIN_RETURN):
    job = 'S';
    break;
  case(DGESVD_MIN_OVERWRITE):
    job = 'O';
    break;
  case(DGESVD_NONE):
    job = 'N';
    break;
  default:
    util_abort("%s: internal error - unrecognized code:%d \n",vector_job);
  }
  return job;
}



/**
   If jobu == DGSEVD_NONE the U matrix can be NULL, same for jobvt.
*/

void matrix_dgesvd(dgesvd_vector_enum jobu , dgesvd_vector_enum jobvt ,  matrix_type * A , double * S , matrix_type * U , matrix_type * VT) {
  char _jobu  = dgesvd_get_vector_job( jobu );
  char _jobvt = dgesvd_get_vector_job( jobvt );
  int m       = matrix_get_rows( A );
  int n       = matrix_get_columns( A );
  int lda     = matrix_get_column_stride( A  );
  int ldu, ldvt;
  double * VT_data , *U_data;
  int info    = 0;
  int min_worksize = util_int_max(3* util_int_min(m , n) + util_int_max(m , n) , 5 * util_int_min(m , n));
  double * work;
  int worksize;


  if (U == NULL) {
    ldu    = 1;
    U_data = NULL;
    if (jobu != DGESVD_NONE)
      util_abort("%s: internal error \n",__func__);
  } else {
    ldu     = matrix_get_column_stride( U  );
    U_data  = matrix_get_data( U );
    if (jobu == DGESVD_NONE)
      util_abort("%s: internal error \n",__func__);
  }

  if (VT == NULL) {
    ldvt    = 1;  /* Will fail if set to zero */
    VT_data = NULL;
    if (jobvt != DGESVD_NONE)
      util_abort("%s: internal error \n",__func__);
  } else {
    ldvt     = matrix_get_column_stride( VT );
    VT_data  = matrix_get_data( VT );
    if (jobvt == DGESVD_NONE)
      util_abort("%s: internal error \n",__func__);
  }

  /*
     Query the routine for optimal worksize.
  */

  work     = util_calloc( 1 , sizeof * work );
  worksize = -1;
  dgesvd_(&_jobu               , /* 1  */
          &_jobvt              , /* 2  */
          &m                   , /* 3  */
          &n                   , /* 4  */
          matrix_get_data( A ) , /* 5  */
          &lda                 , /* 6  */
          S                    , /* 7  */
          U_data               , /* 8  */
          &ldu                 , /* 9  */
          VT_data              , /* 10 */
          &ldvt                , /* 11 */
          work                 , /* 12 */
          &worksize            , /* 13 */
          &info);                /* 14 */


  /* Try to allocate optimal worksize. */
  worksize = (int) work[0];
  double * tmp =  realloc( work , sizeof * work * worksize );
  if (tmp == NULL) {
    /* Could not allocate optimal worksize - settle for the minimum. This can not fail. */
    worksize = min_worksize;
    free(work);
    work = util_calloc( worksize , sizeof * work );
  }else{
    work = tmp; /* The request for optimal worksize succeeded */
  }

  dgesvd_(&_jobu , &_jobvt , &m , &n , matrix_get_data( A ) , &lda , S , U_data , &ldu , VT_data , &ldvt , work , &worksize , &info);
  free( work );
}



/******************************************************************/
/* Eigenvalues of a symmetric matrix                              */
/* Return value is the number of eigenvalues found.               */
/******************************************************************/

int  matrix_dsyevx(bool             compute_eig_vectors ,
                   dsyevx_eig_enum  which_values        , /* DSYEVX | DSYEVX_VALUE_INTERVAL | DSYEVX_INDEX_INTERVAL */
                   dsyevx_uplo_enum uplo,
                   matrix_type    * A ,                   /* The input matrix - is modified by the dsyevx() function. */
                   double VL          ,                   /* Lower limit when using DSYEVX_VALUE_INTERVAL */
                   double VU          ,                   /* Upper limit when using DSYEVX_VALUE_INTERVAL */
                   int    IL          ,                   /* Lower index when using DSYEVX_INDEX_INTERVAL */
                   int    IU          ,                   /* Upper index when using DSYEVX_INDEX_INTERVAL */
                   double *eig_values ,                   /* The calcualated eigenvalues                  */
                   matrix_type * Z    ) {                 /* The eigenvectors as columns vectors          */

  int lda  = matrix_get_column_stride( A );
  int n    = matrix_get_rows( A );
  char jobz;
  char range;
  char uplo_c;

  if (compute_eig_vectors)
    jobz = 'V';
  else
    jobz = 'N';

  switch(which_values) {
  case(DSYEVX_ALL):
    range = 'A';
    break;
  case(DSYEVX_VALUE_INTERVAL):
    range = 'V';
    break;
  case(DSYEVX_INDEX_INTERVAL):
    range = 'I';
    break;
  default:
    util_abort("%s: internal error \n",__func__);
  }

  if (uplo == DSYEVX_AUPPER)
    uplo_c = 'U';
  else if (uplo == DSYEVX_ALOWER)
    uplo_c = 'L';
  else
    util_abort("%s: internal error \n",__func__);


  if (!matrix_is_quadratic( A ))
    util_abort("%s: matrix A must be quadratic \n",__func__);

  {
    int      num_eigenvalues , ldz, info , worksize;
    int    * ifail = util_calloc( n     , sizeof * ifail );
    int    * iwork = util_calloc( 5 * n , sizeof * iwork );
    double * work  = util_calloc( 1     , sizeof * work  );
    double * z_data;
    double   abstol = 0.0; /* SHopuld */


    if (compute_eig_vectors) {
      ldz    = matrix_get_column_stride( Z );
      z_data = matrix_get_data( Z );
    } else {
      /* In this case we can accept that Z == NULL */
      ldz    = 1;
      z_data = NULL;
    }

    /* First call to determine optimal worksize. */
    worksize = -1;
    info     = 0;
    dsyevx_( &jobz,                /*  1 */
             &range,               /*  2 */
             &uplo_c,              /*  3 */
             &n,                   /*  4 */
             matrix_get_data( A ), /*  5 */
             &lda ,                /*  6 */
             &VL ,                 /*  7 */
             &VU ,                 /*  8 */
             &IL ,                 /*  9 */
             &IU ,                 /* 10 */
             &abstol ,             /* 11 */
             &num_eigenvalues ,    /* 12 */
             eig_values ,          /* 13 */
             z_data ,              /* 14 */
             &ldz ,                /* 15 */
             work ,                /* 16 */
             &worksize ,           /* 17 */
             iwork ,               /* 18 */
             ifail ,               /* 19 */
             &info);               /* 20 */


    worksize = (int) work[0];
    {
      double * tmp = realloc(work , sizeof * work * worksize );
      if (tmp == NULL) {
        /*
           OK - we could not get the optimal worksize,
           try again with the minimum.
        */
        worksize = 8 * n;
        work = util_realloc(work , sizeof * work * worksize );
      } else
        work = tmp; /* The request for optimal worksize succeeded */
    }
    /* Second call: do the job */
    info     = 0;
    dsyevx_( &jobz,
             &range,
             &uplo_c,
             &n,
             matrix_get_data( A ),
             &lda ,
             &VL ,
             &VU ,
             &IL ,
             &IU ,
             &abstol ,
             &num_eigenvalues ,
             eig_values ,
             z_data ,
             &ldz ,
             work ,
             &worksize ,
             iwork ,
             ifail ,
             &info);

    free( ifail );
    free( work );
    free( iwork );
    return num_eigenvalues;
  }
}


/**
   Wrapper function to compute all eigenvalues + eigenvectors with the
   matrix_dsyevx() function.
*/

int  matrix_dsyevx_all(dsyevx_uplo_enum uplo,
                       matrix_type    * A ,                   /* The input matrix - is modified by the dsyevx() function. */
                       double *eig_values ,                   /* The calcualated eigenvalues         */
                       matrix_type * Z    ) {                 /* The eigenvectors as columns vectors */
  int num_eigenvalues;
  num_eigenvalues = matrix_dsyevx(true , DSYEVX_ALL , uplo , A , 0,0,0,0, eig_values , Z);
  return num_eigenvalues;

}



/*****************************************************************/
/* Function to compute QR factorization withe the routine dgeqrf */

void matrix_dgeqrf(matrix_type * A , double * tau) {
  int lda       = matrix_get_column_stride( A );
  int m         = matrix_get_rows( A );
  int n         = matrix_get_columns( A );
  double * work = util_calloc(1 , sizeof * work );
  int worksize;
  int info;


  /* Determine optimal worksize. */
  worksize = -1;
  dgeqrf_(&m , &n , matrix_get_data( A ), &lda , tau , work , &worksize , &info);
  if (info != 0)
    util_abort("%s: dgerqf routine failed with info:%d \n",__func__ , info);
  worksize = ( int ) work[0];
  {
    double * tmp = realloc(work , sizeof * work * worksize );
    if (tmp == NULL) {
      /*
         OK - we could not get the optimal worksize,
         try again with the minimum.
      */
      worksize = n;
      work = util_realloc(work , sizeof * work * worksize );
    } else
      work = tmp; /* The request for optimal worksize succeeded */
  }


  /* Second call - do the actual computation. */
  dgeqrf_(&m , &n , matrix_get_data( A ), &lda , tau , work , &worksize , &info);
  if (info != 0)
    util_abort("%s: dgerqf routine failed with info:%d \n",__func__ , info);
  free( work );
}


/**
    Typically to be used after the matrix_dgeqrf() function to construct a orthormal matrix.
*/

void matrix_dorgqr(matrix_type * A , double * tau, int num_reflectors) {  /* num_reflectors == length of tau. */
  int lda       = matrix_get_column_stride( A );
  int m         = matrix_get_rows( A );
  int n         = matrix_get_columns( A );
  double * work = util_malloc(sizeof * work );
  int worksize;
  int info;


  /* Determine optimal worksize. */
  worksize = -1;
  dorgqr_(&m , &n , &num_reflectors , matrix_get_data( A ), &lda , tau , work , &worksize , &info);
  if (info != 0)
    util_abort("%s: dorgqf routine failed with info:%d \n",__func__ , info);
  worksize = ( int ) work[0];
  {
    double * tmp = realloc(work , sizeof * work * worksize );
    if (tmp == NULL) {
      /*
         OK - we could not get the optimal worksize,
         try again with the minimum.
      */
      worksize = n;
      work = util_realloc(work , sizeof * work * worksize );
    } else
      work = tmp; /* The request for optimal worksize succeeded */
  }


  /* Second call - do the actual computation. */
  dorgqr_(&m , &n , &num_reflectors , matrix_get_data( A ), &lda , tau , work , &worksize , &info);
  if (info != 0)
    util_abort("%s: dorqf routine failed with info:%d \n",__func__ , info);
  free( work );
}

/*****************************************************************/

/*****************************************************************/
/* Factorization */

/* Currently only used as 'support' function for the matrix_det function. */
static void matrix_dgetrf__( matrix_type * A, int * ipiv, int * info) {
  int lda       = matrix_get_column_stride( A );
  int m         = matrix_get_rows( A );
  int n         = matrix_get_columns( A );

  dgetrf_( &m , &n , matrix_get_data( A ) , &lda , ipiv , info);
}


/**
   Calculated the determinant of A. The matrix content will be
   destroyed.
*/

double matrix_det( matrix_type *A ) {
  matrix_lapack_assert_square( A );
  {

    int       dgetrf_info;
    double    det       = 1;
    double    det_scale = 0;
    int       n         = matrix_get_columns( A );
    int * ipiv          = util_malloc( n * sizeof * ipiv );
    matrix_dgetrf__( A , ipiv , &dgetrf_info );
    {
      int i;
      for (i=0; i < n; i++) {
        det *= matrix_iget(A , i , i);
        if (det == 0) return 0;   /* Holy fuck - a float == comparison ?? */

        if (ipiv[i] != (i + 1))   /* A permutation has taken place. */
          det *= -1;


        /* Try to avoid overflow/underflow by factoring out the order of magnitude. */
        while (fabs(det) > 10.0) {
          det       /= 10;
          det_scale += 1;
        }

        while (fabs(det) < 1.0) {
          det       *= 10;
          det_scale -= 1;
        }
      }
    }

    free( ipiv );
    return det * pow(10 , det_scale );
  }
}





/*****************************************************************/
/* The matrix will be inverted in-place, the inversion is based on LU
   factorisation in the routine matrix_dgetrf__(  ).

   The return value:

     =0 : Success
     >0 : Singular matrix
     <0 : Invalid input
*/




int matrix_inv( matrix_type * A ) {
  matrix_lapack_assert_square( A );
  {
    int       dgetrf_info;
    int       info;
    int       n         = matrix_get_columns( A );
    int * ipiv          = util_malloc( n * sizeof * ipiv );
    matrix_dgetrf__( A , ipiv , &dgetrf_info );
    {
      int lda  = matrix_get_column_stride( A );
      double * work = util_malloc( sizeof * work );
      int work_size;

      /* First call: determine optimal worksize: */
      work_size = -1;
      dgetri_( &n , matrix_get_data( A ), &lda , ipiv , work , &work_size , &info);

      if (info == 0) {
        work_size = (int) work[0];
        work = util_realloc( work , sizeof * work * work_size );
        dgetri_( &n , matrix_get_data( A ), &lda , ipiv , work , &work_size , &info);
      } else
        util_abort("%s: dgetri_ returned info:%d \n",__func__ , info);

      free( work );
    }
    free( ipiv );
    return info;
  }
}


#ifdef __cplusplus
}
#endif

