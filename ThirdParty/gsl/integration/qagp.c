/* integration/qagp.c
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Brian Gough
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
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>

static int
qagp (const gsl_function *f,
      const double *pts, const size_t npts,
      const double epsabs, const double epsrel, const size_t limit,
      gsl_integration_workspace * workspace,
      double *result, double *abserr,
      gsl_integration_rule * q);

#include "initialise.c"
#include "qpsrt.c"
#include "util.c"
#include "append.c"
#include "reset.c"
#include "qelg.c"
#include "qpsrt2.c"
#include "ptsort.c"
#include "positivity.c"

int
gsl_integration_qagp (const gsl_function *f,
                      double * pts, size_t npts,
                      double epsabs, double epsrel, size_t limit,
                      gsl_integration_workspace * workspace,
                      double * result, double * abserr)
{
  int status = qagp (f, pts, npts,  
                     epsabs, epsrel, limit,
                     workspace,
                     result, abserr,
                     &gsl_integration_qk21) ;
  
  return status ;
}


static int
qagp (const gsl_function * f,
      const double *pts, const size_t npts,
      const double epsabs, const double epsrel, 
      const size_t limit,
      gsl_integration_workspace * workspace,
      double *result, double *abserr,
      gsl_integration_rule * q)
{
  double area, errsum;
  double res_ext, err_ext;
  double result0, abserr0, resabs0;
  double tolerance;

  double ertest = 0;
  double error_over_large_intervals = 0;
  double reseps = 0, abseps = 0, correc = 0;
  size_t ktmin = 0;
  int roundoff_type1 = 0, roundoff_type2 = 0, roundoff_type3 = 0;
  int error_type = 0, error_type2 = 0;

  size_t iteration = 0;

  int positive_integrand = 0;
  int extrapolate = 0;
  int disallow_extrapolation = 0;

  struct extrapolation_table table;

  const size_t nint = npts - 1; /* number of intervals */

  size_t *ndin = workspace->level; /* temporarily alias ndin to level */

  size_t i;

  /* Initialize results */

  *result = 0;
  *abserr = 0;

  /* Test on validity of parameters */

  if (limit > workspace->limit)
    {
      GSL_ERROR ("iteration limit exceeds available workspace", GSL_EINVAL) ;
    }

  if (npts > workspace->limit)
    {
      GSL_ERROR ("npts exceeds size of workspace", GSL_EINVAL);
    }

  if (epsabs <= 0 && (epsrel < 50 * GSL_DBL_EPSILON || epsrel < 0.5e-28))
    {
      GSL_ERROR ("tolerance cannot be achieved with given epsabs and epsrel",
                 GSL_EBADTOL);
    }

  /* Check that the integration range and break points are an
     ascending sequence */

  for (i = 0; i < nint; i++)
    {
      if (pts[i + 1] < pts[i])
        {
          GSL_ERROR ("points are not in an ascending sequence", GSL_EINVAL);
        }
    }

  /* Perform the first integration */

  result0 = 0;
  abserr0 = 0;
  resabs0 = 0;

  initialise (workspace, 0.0, 0.0) ;

  for (i = 0; i < nint; i++)
    {
      double area1, error1, resabs1, resasc1;
      const double a1 = pts[i];
      const double b1 = pts[i + 1];

      q (f, a1, b1, &area1, &error1, &resabs1, &resasc1);

      result0 = result0 + area1;
      abserr0 = abserr0 + error1;
      resabs0 = resabs0 + resabs1;

      append_interval (workspace, a1, b1, area1, error1);

      if (error1 == resasc1 && error1 != 0.0)
        {
          ndin[i] = 1;
        }
      else
        {
          ndin[i] = 0;
        }
    }

  /* Compute the initial error estimate */

  errsum = 0.0;

  for (i = 0; i < nint; i++)
    {
      if (ndin[i])
        {
          workspace->elist[i] = abserr0;
        }

      errsum = errsum + workspace->elist[i];

    }

  for (i = 0; i < nint; i++)
    {
      workspace->level[i] = 0;
    }

  /* Sort results into order of decreasing error via the indirection
     array order[] */

  sort_results (workspace);

  /* Test on accuracy */

  tolerance = GSL_MAX_DBL (epsabs, epsrel * fabs (result0));

  if (abserr0 <= 100 * GSL_DBL_EPSILON * resabs0 && abserr0 > tolerance)
    {
      *result = result0;
      *abserr = abserr0;

      GSL_ERROR ("cannot reach tolerance because of roundoff error"
                 "on first attempt", GSL_EROUND);
    }
  else if (abserr0 <= tolerance)
    {
      *result = result0;
      *abserr = abserr0;

      return GSL_SUCCESS;
    }
  else if (limit == 1)
    {
      *result = result0;
      *abserr = abserr0;

      GSL_ERROR ("a maximum of one iteration was insufficient", GSL_EMAXITER);
    }

  /* Initialization */

  initialise_table (&table);
  append_table (&table, result0);

  area = result0;

  res_ext = result0;
  err_ext = GSL_DBL_MAX;

  error_over_large_intervals = errsum;
  ertest = tolerance;

  positive_integrand = test_positivity (result0, resabs0);

  iteration = nint - 1; 

  do
    {
      size_t current_level;
      double a1, b1, a2, b2;
      double a_i, b_i, r_i, e_i;
      double area1 = 0, area2 = 0, area12 = 0;
      double error1 = 0, error2 = 0, error12 = 0;
      double resasc1, resasc2;
      double resabs1, resabs2;
      double last_e_i;

      /* Bisect the subinterval with the largest error estimate */

      retrieve (workspace, &a_i, &b_i, &r_i, &e_i);

      current_level = workspace->level[workspace->i] + 1;

      a1 = a_i;
      b1 = 0.5 * (a_i + b_i);
      a2 = b1;
      b2 = b_i;

      iteration++;

      q (f, a1, b1, &area1, &error1, &resabs1, &resasc1);
      q (f, a2, b2, &area2, &error2, &resabs2, &resasc2);

      area12 = area1 + area2;
      error12 = error1 + error2;
      last_e_i = e_i;

      /* Improve previous approximations to the integral and test for
         accuracy.

         We write these expressions in the same way as the original
         QUADPACK code so that the rounding errors are the same, which
         makes testing easier. */

      errsum = errsum + error12 - e_i;
      area = area + area12 - r_i;

      tolerance = GSL_MAX_DBL (epsabs, epsrel * fabs (area));

      if (resasc1 != error1 && resasc2 != error2)
        {
          double delta = r_i - area12;

          if (fabs (delta) <= 1.0e-5 * fabs (area12) && error12 >= 0.99 * e_i)
            {
              if (!extrapolate)
                {
                  roundoff_type1++;
                }
              else
                {
                  roundoff_type2++;
                }
            }

          if (i > 10 && error12 > e_i)
            {
              roundoff_type3++;
            }
        }

      /* Test for roundoff and eventually set error flag */

      if (roundoff_type1 + roundoff_type2 >= 10 || roundoff_type3 >= 20)
        {
          error_type = 2;       /* round off error */
        }

      if (roundoff_type2 >= 5)
        {
          error_type2 = 1;
        }

      /* set error flag in the case of bad integrand behaviour at
         a point of the integration range */

      if (subinterval_too_small (a1, a2, b2))
        {
          error_type = 4;
        }

      /* append the newly-created intervals to the list */

      update (workspace, a1, b1, area1, error1, a2, b2, area2, error2);

      if (errsum <= tolerance)
        {
          goto compute_result;
        }

      if (error_type)
        {
          break;
        }

      if (iteration >= limit - 1)
        {
          error_type = 1;
          break;
        }

      if (disallow_extrapolation)
        {
          continue;
        }

      error_over_large_intervals += -last_e_i;

      if (current_level < workspace->maximum_level)
        {
          error_over_large_intervals += error12;
        }

      if (!extrapolate)
        {
          /* test whether the interval to be bisected next is the
             smallest interval. */
          if (large_interval (workspace))
            continue;

          extrapolate = 1;
          workspace->nrmax = 1;
        }

      /* The smallest interval has the largest error.  Before
         bisecting decrease the sum of the errors over the larger
         intervals (error_over_large_intervals) and perform
         extrapolation. */

      if (!error_type2 && error_over_large_intervals > ertest)
        {
          if (increase_nrmax (workspace))
            continue;
        }

      /* Perform extrapolation */

      append_table (&table, area);

      if (table.n < 3) 
        {
          goto skip_extrapolation;
        } 

      qelg (&table, &reseps, &abseps);

      ktmin++;

      if (ktmin > 5 && err_ext < 0.001 * errsum)
        {
          error_type = 5;
        }

      if (abseps < err_ext)
        {
          ktmin = 0;
          err_ext = abseps;
          res_ext = reseps;
          correc = error_over_large_intervals;
          ertest = GSL_MAX_DBL (epsabs, epsrel * fabs (reseps));
          if (err_ext <= ertest)
            break;
        }

      /* Prepare bisection of the smallest interval. */

      if (table.n == 1)
        {
          disallow_extrapolation = 1;
        }

      if (error_type == 5)
        {
          break;
        }

    skip_extrapolation:

      reset_nrmax (workspace);
      extrapolate = 0;
      error_over_large_intervals = errsum;

    }
  while (iteration < limit);

  *result = res_ext;
  *abserr = err_ext;

  if (err_ext == GSL_DBL_MAX)
    goto compute_result;

  if (error_type || error_type2)
    {
      if (error_type2)
        {
          err_ext += correc;
        }

      if (error_type == 0)
        error_type = 3;

      if (result != 0 && area != 0)
        {
          if (err_ext / fabs (res_ext) > errsum / fabs (area))
            goto compute_result;
        }
      else if (err_ext > errsum)
        {
          goto compute_result;
        }
      else if (area == 0.0)
        {
          goto return_error;
        }
    }

  /*  Test on divergence. */

  {
    double max_area = GSL_MAX_DBL (fabs (res_ext), fabs (area));

    if (!positive_integrand && max_area < 0.01 * resabs0)
      goto return_error;
  }

  {
    double ratio = res_ext / area;

    if (ratio < 0.01 || ratio > 100 || errsum > fabs (area))
      error_type = 6;
  }

  goto return_error;

compute_result:

  *result = sum_results (workspace);
  *abserr = errsum;

return_error:

  if (error_type > 2)
    error_type--;

  if (error_type == 0)
    {
      return GSL_SUCCESS;
    }
  else if (error_type == 1)
    {
      GSL_ERROR ("number of iterations was insufficient", GSL_EMAXITER);
    }
  else if (error_type == 2)
    {
      GSL_ERROR ("cannot reach tolerance because of roundoff error",
                 GSL_EROUND);
    }
  else if (error_type == 3)
    {
      GSL_ERROR ("bad integrand behavior found in the integration interval",
                 GSL_ESING);
    }
  else if (error_type == 4)
    {
      GSL_ERROR ("roundoff error detected in the extrapolation table",
                 GSL_EROUND);
    }
  else if (error_type == 5)
    {
      GSL_ERROR ("integral is divergent, or slowly convergent",
                 GSL_EDIVERGE);
    }
  else
    {
      GSL_ERROR ("could not integrate function", GSL_EFAILED);
    }
}
