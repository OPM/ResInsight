/*
  Copyright 2010 SINTEF ICT, Applied Mathematics.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPM_BLAS_LAPACK_HEADER_INCLUDED
#define OPM_BLAS_LAPACK_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MATLAB_MEX_FILE) && MATLAB_MEX_FILE
#include <mex.h>
#undef  MAT_SIZE_T
#define MAT_SIZE_T mwSignedIndex
#endif

#ifndef MAT_SIZE_T
#define MAT_SIZE_T int
#endif


/* C <- a1*op(A)*op(B) + a2*C  where  op(X) in {X, X.'} */
void dgemm_(const char *transA  , const char *transB   ,
            const MAT_SIZE_T*  m, const MAT_SIZE_T* n  , const MAT_SIZE_T* k  ,
            const double*     a1, const double*     A  , const MAT_SIZE_T* ldA,
            const double*      B, const MAT_SIZE_T* ldB,
            const double*     a2,       double*     C  , const MAT_SIZE_T* ldC);


/* C <- a1*A*A' + a2*C   *or*   C <- a1*A'*A + a2*C */
void dsyrk_(const char       *uplo, const char       *trans,
            const MAT_SIZE_T *n   , const MAT_SIZE_T *k    ,
            const double     *a1  , const double     *A    , const MAT_SIZE_T *ldA,
            const double     *a2  ,       double     *C    , const MAT_SIZE_T *ldC);


void dgeqrf_(const MAT_SIZE_T *m    , const MAT_SIZE_T *n   ,
                   double     *A    , const MAT_SIZE_T *ld  ,
                   double     *tau  ,       double     *work,
             const MAT_SIZE_T *lwork,       MAT_SIZE_T *info);


void dorgqr_(const MAT_SIZE_T *m   , const MAT_SIZE_T *n    , const MAT_SIZE_T *k  ,
                   double     *A   , const MAT_SIZE_T *ld   , const double     *tau,
                   double     *work, const MAT_SIZE_T *lwork,       MAT_SIZE_T *info);

/* A <- LU(A) */
void dgetrf_(const MAT_SIZE_T *m   , const MAT_SIZE_T *n ,
             double           *A   , const MAT_SIZE_T *ld,
             MAT_SIZE_T       *ipiv, MAT_SIZE_T       *info);

/* B <- A \ B, when A is LU(A) from dgetrf() */
void dgetrs_(const char       *trans, const MAT_SIZE_T *n,
             const MAT_SIZE_T *nrhs ,
             const double     *A    , const MAT_SIZE_T *lda,
             const MAT_SIZE_T *ipiv , double           *B,
             const MAT_SIZE_T *ldb  , MAT_SIZE_T       *info);

/* B <- A \ B, tridiagonal A with bands DL, D, DU */
void dgtsv_(const MAT_SIZE_T *n    ,
            const MAT_SIZE_T *nrhs ,
                  double     *DL   ,
                  double     *D    ,
                  double     *DU   ,
                  double     *B    ,
            const MAT_SIZE_T *ldb  ,
            MAT_SIZE_T       *info);

/* B <- A \ B, band matrix A stored in AB with kl subdiagonals, ku superdiagonals */
void dgbsv_(const MAT_SIZE_T *n    ,
            const MAT_SIZE_T *kl   ,
            const MAT_SIZE_T *ku   ,
            const MAT_SIZE_T *nrhs ,
            double     *AB   ,
            const MAT_SIZE_T *ldab ,
            MAT_SIZE_T *ipiv ,
            double     *B    ,
            const MAT_SIZE_T *ldb  ,
            MAT_SIZE_T       *info);

/* B <- A \ B, general solver */
void dgesv_(const MAT_SIZE_T *n,
            const MAT_SIZE_T *nrhs ,
            double     *A   ,
            const MAT_SIZE_T *lda ,
            MAT_SIZE_T *piv ,
            double     *B    ,
            const MAT_SIZE_T *ldb  ,
            MAT_SIZE_T *info);

/* A <- chol(A) */
void dpotrf_(const char *uplo, const MAT_SIZE_T *n,
             double     *A   , const MAT_SIZE_T *lda,
             MAT_SIZE_T *info);

/* B <- (A \ (A' \ B)), when A=DPOTRF(A_orig) */
void dpotrs_(const char *uplo, const MAT_SIZE_T *n  , const MAT_SIZE_T *nrhs,
             double     *A   , const MAT_SIZE_T *lda,
             double     *B   , const MAT_SIZE_T *ldb,       MAT_SIZE_T *info);

/* A <- chol(A), packed format. */
void dpptrf_(const char *uplo, const MAT_SIZE_T *n,
             double     *Ap  ,       MAT_SIZE_T *info);

/* A <- (A \ (A' \ eye(n)) when A=DPPTRF(A_orig) (packed format). */
void dpptri_(const char *uplo, const MAT_SIZE_T *n,
             double     *Ap  ,       MAT_SIZE_T *info);

/* y <- a1*op(A)*x + a2*y */
void dgemv_(const char       *trans,
            const MAT_SIZE_T *m    , const MAT_SIZE_T *n,
            const double     *a1   , const double     *A, const MAT_SIZE_T *ldA ,
                                     const double     *x, const MAT_SIZE_T *incX,
            const double     *a2   ,       double     *y, const MAT_SIZE_T *incY);


/* y <- a*x + y */
void daxpy_(const MAT_SIZE_T *n, const double *a,
            const double *x, const MAT_SIZE_T *incx,
                  double *y, const MAT_SIZE_T *incy);

/* s <- x' * y */
double ddot_(const MAT_SIZE_T *n, const double *x, const MAT_SIZE_T *incx,
                                  const double *y, const MAT_SIZE_T *incy);



#ifdef __cplusplus
}
#endif

#endif /* OPM_BLAS_LAPACK_HEADER_INCLUDED */
