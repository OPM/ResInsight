#include <gsl/gsl_math.h>
#include <gsl/gsl_cblas.h>
#include "cblas.h"
#include "error_cblas_l3.h"

void
cblas_zher2k (const enum CBLAS_ORDER Order, const enum CBLAS_UPLO Uplo,
              const enum CBLAS_TRANSPOSE Trans, const int N, const int K,
              const void *alpha, const void *A, const int lda, const void *B,
              const int ldb, const double beta, void *C, const int ldc)
{
#define BASE double
#include "source_her2k.h"
#undef BASE
}
