/* linalg/qrpt.c
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Gerard Jungman, Brian Gough
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permute_vector.h>
#include <gsl/gsl_blas.h>

#include <gsl/gsl_linalg.h>

#include "apply_givens.c"

/* Factorise a general M x N matrix A into
 *
 *   A P = Q R
 *
 * where Q is orthogonal (M x M) and R is upper triangular (M x N).
 * When A is rank deficient, r = rank(A) < n, then the permutation is
 * used to ensure that the lower n - r rows of R are zero and the first
 * r columns of Q form an orthonormal basis for A.
 *
 * Q is stored as a packed set of Householder transformations in the
 * strict lower triangular part of the input matrix.
 *
 * R is stored in the diagonal and upper triangle of the input matrix.
 *
 * P: column j of P is column k of the identity matrix, where k =
 * permutation->data[j]
 *
 * The full matrix for Q can be obtained as the product
 *
 *       Q = Q_k .. Q_2 Q_1
 *
 * where k = MIN(M,N) and
 *
 *       Q_i = (I - tau_i * v_i * v_i')
 *
 * and where v_i is a Householder vector
 *
 *       v_i = [1, m(i+1,i), m(i+2,i), ... , m(M,i)]
 *
 * This storage scheme is the same as in LAPACK.  See LAPACK's
 * dgeqpf.f for details.
 * 
 */

int
gsl_linalg_QRPT_decomp (gsl_matrix * A, gsl_vector * tau, gsl_permutation * p, int *signum, gsl_vector * norm)
{
  const size_t M = A->size1;
  const size_t N = A->size2;

  if (tau->size != GSL_MIN (M, N))
    {
      GSL_ERROR ("size of tau must be MIN(M,N)", GSL_EBADLEN);
    }
  else if (p->size != N)
    {
      GSL_ERROR ("permutation size must be N", GSL_EBADLEN);
    }
  else if (norm->size != N)
    {
      GSL_ERROR ("norm size must be N", GSL_EBADLEN);
    }
  else
    {
      size_t i;

      *signum = 1;

      gsl_permutation_init (p); /* set to identity */

      /* Compute column norms and store in workspace */

      for (i = 0; i < N; i++)
        {
          gsl_vector_view c = gsl_matrix_column (A, i);
          double x = gsl_blas_dnrm2 (&c.vector);
          gsl_vector_set (norm, i, x);
        }

      for (i = 0; i < GSL_MIN (M, N); i++)
        {
          /* Bring the column of largest norm into the pivot position */

          double max_norm = gsl_vector_get(norm, i);
          size_t j, kmax = i;

          for (j = i + 1; j < N; j++)
            {
              double x = gsl_vector_get (norm, j);

              if (x > max_norm)
                {
                  max_norm = x;
                  kmax = j;
                }
            }

          if (kmax != i)
            {
              gsl_matrix_swap_columns (A, i, kmax);
              gsl_permutation_swap (p, i, kmax);
              gsl_vector_swap_elements(norm,i,kmax);

              (*signum) = -(*signum);
            }

          /* Compute the Householder transformation to reduce the j-th
             column of the matrix to a multiple of the j-th unit vector */

          {
            gsl_vector_view c_full = gsl_matrix_column (A, i);
            gsl_vector_view c = gsl_vector_subvector (&c_full.vector, 
                                                      i, M - i);
            double tau_i = gsl_linalg_householder_transform (&c.vector);

            gsl_vector_set (tau, i, tau_i);

            /* Apply the transformation to the remaining columns */

            if (i + 1 < N)
              {
                gsl_matrix_view m = gsl_matrix_submatrix (A, i, i + 1, M - i, N - (i+1));

                gsl_linalg_householder_hm (tau_i, &c.vector, &m.matrix);
              }
          }

          /* Update the norms of the remaining columns too */

          if (i + 1 < M) 
            {
              for (j = i + 1; j < N; j++)
                {
                  double x = gsl_vector_get (norm, j);

                  if (x > 0.0)
                    {
                      double y = 0;
                      double temp= gsl_matrix_get (A, i, j) / x;
                  
                      if (fabs (temp) >= 1)
                        y = 0.0;
                      else
                        y = x * sqrt (1 - temp * temp);
                      
                      /* recompute norm to prevent loss of accuracy */

                      if (fabs (y / x) < sqrt (20.0) * GSL_SQRT_DBL_EPSILON)
                        {
                          gsl_vector_view c_full = gsl_matrix_column (A, j);
                          gsl_vector_view c = 
                            gsl_vector_subvector(&c_full.vector,
                                                 i+1, M - (i+1));
                          y = gsl_blas_dnrm2 (&c.vector);
                        }
                  
                      gsl_vector_set (norm, j, y);
                    }
                }
            }
        }

      return GSL_SUCCESS;
    }
}

int
gsl_linalg_QRPT_decomp2 (const gsl_matrix * A, gsl_matrix * q, gsl_matrix * r, gsl_vector * tau, gsl_permutation * p, int *signum, gsl_vector * norm)
{
  const size_t M = A->size1;
  const size_t N = A->size2;

  if (q->size1 != M || q->size2 !=M) 
    {
      GSL_ERROR ("q must be M x M", GSL_EBADLEN);
    }
  else if (r->size1 != M || r->size2 !=N)
    {
      GSL_ERROR ("r must be M x N", GSL_EBADLEN);
    }
  else if (tau->size != GSL_MIN (M, N))
    {
      GSL_ERROR ("size of tau must be MIN(M,N)", GSL_EBADLEN);
    }
  else if (p->size != N)
    {
      GSL_ERROR ("permutation size must be N", GSL_EBADLEN);
    }
  else if (norm->size != N)
    {
      GSL_ERROR ("norm size must be N", GSL_EBADLEN);
    }

  gsl_matrix_memcpy (r, A);

  gsl_linalg_QRPT_decomp (r, tau, p, signum, norm);

  /* FIXME:  aliased arguments depends on behavior of unpack routine! */

  gsl_linalg_QR_unpack (r, tau, q, r);

  return GSL_SUCCESS;
}


/* Solves the system A x = b using the Q R P^T factorisation,

   R z = Q^T b

   x = P z;

   to obtain x. Based on SLATEC code. */

int
gsl_linalg_QRPT_solve (const gsl_matrix * QR,
                       const gsl_vector * tau,
                       const gsl_permutation * p,
                       const gsl_vector * b,
                       gsl_vector * x)
{
  if (QR->size1 != QR->size2)
    {
      GSL_ERROR ("QR matrix must be square", GSL_ENOTSQR);
    }
  else if (QR->size1 != p->size)
    {
      GSL_ERROR ("matrix size must match permutation size", GSL_EBADLEN);
    }
  else if (QR->size1 != b->size)
    {
      GSL_ERROR ("matrix size must match b size", GSL_EBADLEN);
    }
  else if (QR->size2 != x->size)
    {
      GSL_ERROR ("matrix size must match solution size", GSL_EBADLEN);
    }
  else
    {
      gsl_vector_memcpy (x, b);

      gsl_linalg_QRPT_svx (QR, tau, p, x);
      
      return GSL_SUCCESS;
    }
}

int
gsl_linalg_QRPT_svx (const gsl_matrix * QR,
                     const gsl_vector * tau,
                     const gsl_permutation * p,
                     gsl_vector * x)
{
  if (QR->size1 != QR->size2)
    {
      GSL_ERROR ("QR matrix must be square", GSL_ENOTSQR);
    }
  else if (QR->size1 != p->size)
    {
      GSL_ERROR ("matrix size must match permutation size", GSL_EBADLEN);
    }
  else if (QR->size2 != x->size)
    {
      GSL_ERROR ("matrix size must match solution size", GSL_EBADLEN);
    }
  else
    {
      /* compute sol = Q^T b */

      gsl_linalg_QR_QTvec (QR, tau, x);

      /* Solve R x = sol, storing x inplace in sol */

      gsl_blas_dtrsv (CblasUpper, CblasNoTrans, CblasNonUnit, QR, x);

      gsl_permute_vector_inverse (p, x);

      return GSL_SUCCESS;
    }
}

/* Find the least squares solution to the overdetermined system 
 *
 *   A x = b 
 *  
 * for M >= N using the QRPT factorization A P = Q R. Assumes
 * A has full column rank.
 */

int
gsl_linalg_QRPT_lssolve (const gsl_matrix * QR, const gsl_vector * tau, const gsl_permutation * p,
                         const gsl_vector * b, gsl_vector * x, gsl_vector * residual)
{
  const size_t N = QR->size2;
  int status = gsl_linalg_QRPT_lssolve2(QR, tau, p, b, N, x, residual);
  return status;
}

/* Find the least squares solution to the overdetermined system 
 *
 *   A x = b 
 *  
 * for M >= N using the QRPT factorization A P = Q R, where A
 * is assumed rank deficient with a given rank.
 */

int
gsl_linalg_QRPT_lssolve2 (const gsl_matrix * QR, const gsl_vector * tau, const gsl_permutation * p,
                          const gsl_vector * b, const size_t rank, gsl_vector * x, gsl_vector * residual)
{
  const size_t M = QR->size1;
  const size_t N = QR->size2;

  if (M < N)
    {
      GSL_ERROR ("QR matrix must have M>=N", GSL_EBADLEN);
    }
  else if (M != b->size)
    {
      GSL_ERROR ("matrix size must match b size", GSL_EBADLEN);
    }
  else if (rank == 0 || rank > N)
    {
      GSL_ERROR ("rank must have 0 < rank <= N", GSL_EBADLEN);
    }
  else if (N != x->size)
    {
      GSL_ERROR ("matrix size must match solution size", GSL_EBADLEN);
    }
  else if (M != residual->size)
    {
      GSL_ERROR ("matrix size must match residual size", GSL_EBADLEN);
    }
  else
    {
      gsl_matrix_const_view R11 = gsl_matrix_const_submatrix (QR, 0, 0, rank, rank);
      gsl_vector_view QTb1 = gsl_vector_subvector(residual, 0, rank);
      gsl_vector_view x1 = gsl_vector_subvector(x, 0, rank);
      size_t i;

      /* compute work = Q^T b */
      gsl_vector_memcpy(residual, b);
      gsl_linalg_QR_QTvec (QR, tau, residual);

      /* solve R_{11} x(1:r) = [Q^T b](1:r) */
      gsl_vector_memcpy(&(x1.vector), &(QTb1.vector));
      gsl_blas_dtrsv (CblasUpper, CblasNoTrans, CblasNonUnit, &(R11.matrix), &(x1.vector));

      /* x(r+1:N) = 0 */
      for (i = rank; i < N; ++i)
        gsl_vector_set(x, i, 0.0);

      /* compute x = P y */
      gsl_permute_vector_inverse (p, x);

      /* compute residual = b - A x = Q (Q^T b - R x) */
      gsl_vector_set_zero(&(QTb1.vector));
      gsl_linalg_QR_Qvec(QR, tau, residual);

      return GSL_SUCCESS;
    }
}

int
gsl_linalg_QRPT_QRsolve (const gsl_matrix * Q, const gsl_matrix * R,
                         const gsl_permutation * p,
                         const gsl_vector * b,
                         gsl_vector * x)
{
  if (Q->size1 != Q->size2 || R->size1 != R->size2)
    {
      return GSL_ENOTSQR;
    }
  else if (Q->size1 != p->size || Q->size1 != R->size1
           || Q->size1 != b->size)
    {
      return GSL_EBADLEN;
    }
  else
    {
      /* compute b' = Q^T b */

      gsl_blas_dgemv (CblasTrans, 1.0, Q, b, 0.0, x);

      /* Solve R x = b', storing x inplace */

      gsl_blas_dtrsv (CblasUpper, CblasNoTrans, CblasNonUnit, R, x);

      /* Apply permutation to solution in place */

      gsl_permute_vector_inverse (p, x);

      return GSL_SUCCESS;
    }
}

int
gsl_linalg_QRPT_Rsolve (const gsl_matrix * QR,
                        const gsl_permutation * p,
                        const gsl_vector * b,
                        gsl_vector * x)
{
  if (QR->size1 != QR->size2)
    {
      GSL_ERROR ("QR matrix must be square", GSL_ENOTSQR);
    }
  else if (QR->size1 != b->size)
    {
      GSL_ERROR ("matrix size must match b size", GSL_EBADLEN);
    }
  else if (QR->size2 != x->size)
    {
      GSL_ERROR ("matrix size must match x size", GSL_EBADLEN);
    }
  else if (p->size != x->size)
    {
      GSL_ERROR ("permutation size must match x size", GSL_EBADLEN);
    }
  else
    {
      /* Copy x <- b */

      gsl_vector_memcpy (x, b);

      /* Solve R x = b, storing x inplace */

      gsl_blas_dtrsv (CblasUpper, CblasNoTrans, CblasNonUnit, QR, x);

      gsl_permute_vector_inverse (p, x);

      return GSL_SUCCESS;
    }
}


int
gsl_linalg_QRPT_Rsvx (const gsl_matrix * QR,
                      const gsl_permutation * p,
                      gsl_vector * x)
{
  if (QR->size1 != QR->size2)
    {
      GSL_ERROR ("QR matrix must be square", GSL_ENOTSQR);
    }
  else if (QR->size2 != x->size)
    {
      GSL_ERROR ("matrix size must match x size", GSL_EBADLEN);
    }
  else if (p->size != x->size)
    {
      GSL_ERROR ("permutation size must match x size", GSL_EBADLEN);
    }
  else
    {
      /* Solve R x = b, storing x inplace */

      gsl_blas_dtrsv (CblasUpper, CblasNoTrans, CblasNonUnit, QR, x);

      gsl_permute_vector_inverse (p, x);

      return GSL_SUCCESS;
    }
}



/* Update a Q R P^T factorisation for A P= Q R ,  A' = A + u v^T,

   Q' R' P^-1 = QR P^-1 + u v^T
   = Q (R + Q^T u v^T P ) P^-1
   = Q (R + w v^T P) P^-1

   where w = Q^T u.

   Algorithm from Golub and Van Loan, "Matrix Computations", Section
   12.5 (Updating Matrix Factorizations, Rank-One Changes)  */

int
gsl_linalg_QRPT_update (gsl_matrix * Q, gsl_matrix * R,
                        const gsl_permutation * p,
                        gsl_vector * w, const gsl_vector * v)
{
  const size_t M = R->size1;
  const size_t N = R->size2;

  if (Q->size1 != M || Q->size2 != M)
    {
      GSL_ERROR ("Q matrix must be M x M if R is M x N", GSL_ENOTSQR);
    }
  else if (w->size != M)
    {
      GSL_ERROR ("w must be length M if R is M x N", GSL_EBADLEN);
    }
  else if (v->size != N)
    {
      GSL_ERROR ("v must be length N if R is M x N", GSL_EBADLEN);
    }
  else
    {
      size_t j, k;
      double w0;

      /* Apply Given's rotations to reduce w to (|w|, 0, 0, ... , 0) 

         J_1^T .... J_(n-1)^T w = +/- |w| e_1

         simultaneously applied to R,  H = J_1^T ... J^T_(n-1) R
         so that H is upper Hessenberg.  (12.5.2) */

      for (k = M - 1; k > 0; k--)
        {
          double c, s;
          double wk = gsl_vector_get (w, k);
          double wkm1 = gsl_vector_get (w, k - 1);

          gsl_linalg_givens (wkm1, wk, &c, &s);
          gsl_linalg_givens_gv (w, k - 1, k, c, s);
          apply_givens_qr (M, N, Q, R, k - 1, k, c, s);
        }

      w0 = gsl_vector_get (w, 0);

      /* Add in w v^T  (Equation 12.5.3) */

      for (j = 0; j < N; j++)
        {
          double r0j = gsl_matrix_get (R, 0, j);
          size_t p_j = gsl_permutation_get (p, j);
          double vj = gsl_vector_get (v, p_j);
          gsl_matrix_set (R, 0, j, r0j + w0 * vj);
        }

      /* Apply Givens transformations R' = G_(n-1)^T ... G_1^T H  
         Equation 12.5.4 */

     for (k = 1; k < GSL_MIN(M,N+1); k++)
        {
          double c, s;
          double diag = gsl_matrix_get (R, k - 1, k - 1);
          double offdiag = gsl_matrix_get (R, k, k - 1);

          gsl_linalg_givens (diag, offdiag, &c, &s);
          apply_givens_qr (M, N, Q, R, k - 1, k, c, s);

          gsl_matrix_set (R, k, k - 1, 0.0);    /* exact zero of G^T */
        }

      return GSL_SUCCESS;
    }
}

/*
gsl_linalg_QRPT_rank()
  Estimate rank of triangular matrix from QRPT decomposition

Inputs: QR  - QRPT decomposed matrix
        tol - tolerance for rank determination; if < 0,
              a default value is used:
              20 * (M + N) * eps(max(|diag(R)|))

Return: rank estimate
*/

size_t
gsl_linalg_QRPT_rank (const gsl_matrix * QR, const double tol)
{
  const size_t M = QR->size1;
  const size_t N = QR->size2;
  gsl_vector_const_view diag = gsl_matrix_const_diagonal(QR);
  double eps;
  size_t i;
  size_t r = 0;

  if (tol < 0.0)
    {
      double min, max, absmax;
      int ee;

      gsl_vector_minmax(&diag.vector, &min, &max);
      absmax = GSL_MAX(fabs(min), fabs(max));
      ee = (int) (log(absmax) / M_LN2); /* can't use log2 since its not ANSI */

      eps = 20.0 * (M + N) * pow(2.0, (double) ee) * GSL_DBL_EPSILON;
    }
  else
    eps = tol;

  /* count number of diagonal elements with |di| > eps */
  for (i = 0; i < GSL_MIN(M, N); ++i)
    {
      double di = gsl_vector_get(&diag.vector, i);
      if (fabs(di) > eps)
        ++r;
    }

  return r;
}

int
gsl_linalg_QRPT_rcond(const gsl_matrix * QR, double * rcond, gsl_vector * work)
{
  const size_t M = QR->size1;
  const size_t N = QR->size2;

  if (M < N)
    {
      GSL_ERROR ("M must be >= N", GSL_EBADLEN);
    }
  else if (work->size != 3 * N)
    {
      GSL_ERROR ("work vector must have length 3*N", GSL_EBADLEN);
    }
  else
    {
      gsl_matrix_const_view R = gsl_matrix_const_submatrix (QR, 0, 0, N, N);
      int status;

      status = gsl_linalg_tri_upper_rcond(&R.matrix, rcond, work);

      return status;
    }
}
