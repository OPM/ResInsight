#include <gsl/gsl_math.h>
#include <gsl/gsl_cblas.h>
#include "cblas.h"
#include "error_cblas_l2.h"

void
cblas_cgbmv (const enum CBLAS_ORDER order, const enum CBLAS_TRANSPOSE TransA,
             const int M, const int N, const int KL, const int KU,
             const void *alpha, const void *A, const int lda, const void *X,
             const int incX, const void *beta, void *Y, const int incY)
{
#define BASE float
#include "source_gbmv_c.h"
#undef BASE
}
