#include <gsl/gsl_math.h>
#include <gsl/gsl_cblas.h>
#include "cblas.h"
#include "error_cblas_l3.h"

void
cblas_csyrk (const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
             const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
             const void *alpha, const void *A, const int lda,
             const void *beta, void *C, const int ldc)
{
#define BASE float
#include "source_syrk_c.h"
#undef BASE
}
