#define beale_N         3
#define beale_P         2

static double beale_x0[beale_P] = { 1.0, 1.0 };
static double beale_epsrel = 1.0e-12;

static double beale_Y[beale_N] = { 1.5, 2.25, 2.625 };

static void
beale_checksol(const double x[], const double sumsq,
               const double epsrel, const char *sname,
               const char *pname)
{
  size_t i;
  const double sumsq_exact = 0.0;
  const double beale_x[beale_P] = { 3.0, 0.5 };

  gsl_test_rel(sumsq, sumsq_exact, epsrel, "%s/%s sumsq",
               sname, pname);

  for (i = 0; i < beale_P; ++i)
    {
      gsl_test_rel(x[i], beale_x[i], epsrel, "%s/%s i=%zu",
                   sname, pname, i);
    }
}

static int
beale_f (const gsl_vector * x, void *params, gsl_vector * f)
{
  double x1 = gsl_vector_get(x, 0);
  double x2 = gsl_vector_get(x, 1);
  size_t i;

  for (i = 0; i < beale_N; ++i)
    {
      double yi = beale_Y[i];
      double term = pow(x2, i + 1.0);
      double fi = yi - x1*(1.0 - term);
      gsl_vector_set(f, i, fi);
    }

  (void)params; /* avoid unused parameter warning */

  return GSL_SUCCESS;
}

static int
beale_df (const gsl_vector * x, void *params, gsl_matrix * J)
{
  double x1 = gsl_vector_get(x, 0);
  double x2 = gsl_vector_get(x, 1);
  size_t i;

  for (i = 0; i < beale_N; ++i)
    {
      double term = pow(x2, (double) i);

      gsl_matrix_set(J, i, 0, term*x2 - 1.0);
      gsl_matrix_set(J, i, 1, (i + 1.0) * x1 * term);
    }

  (void)params; /* avoid unused parameter warning */

  return GSL_SUCCESS;
}

static int
beale_fvv (const gsl_vector * x, const gsl_vector * v,
           void *params, gsl_vector * fvv)
{
  double x1 = gsl_vector_get(x, 0);
  double x2 = gsl_vector_get(x, 1);
  double v1 = gsl_vector_get(v, 0);
  double v2 = gsl_vector_get(v, 1);
  size_t i;

  for (i = 0; i < beale_N; ++i)
    {
      double term = pow(x2, (double)i - 1.0);

      gsl_vector_set(fvv, i, (i + 1.0) * v2 * term *
                             (i * v2 * x1 + 2.0 * v1 * x2));
    }

  (void)params; /* avoid unused parameter warning */

  return GSL_SUCCESS;
}

static gsl_multifit_nlinear_fdf beale_func =
{
  beale_f,
  beale_df,
  beale_fvv,
  beale_N,
  beale_P,
  NULL,
  0,
  0,
  0
};

static test_fdf_problem beale_problem =
{
  "beale",
  beale_x0,
  NULL,
  NULL,
  &beale_epsrel,
  &beale_checksol,
  &beale_func
};
