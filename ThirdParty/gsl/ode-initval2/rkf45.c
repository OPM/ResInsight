/* ode-initval2/rkf45.c
 * 
 * Copyright (C) 2001, 2004, 2007 Brian Gough
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

/* Runge-Kutta-Fehlberg 4(5)*/

/* Reference eg. Hairer, E., Norsett S.P., Wanner, G. Solving ordinary
   differential equations I, Nonstiff Problems, 2nd revised edition,
   Springer, 2000.
*/

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include "odeiv_util.h"
#include "step_utils.c"

/* Runge-Kutta-Fehlberg coefficients. Zero elements left out */

static const double ah[] =
  { 1.0 / 4.0, 3.0 / 8.0, 12.0 / 13.0, 1.0, 1.0 / 2.0 };
static const double b3[] = { 3.0 / 32.0, 9.0 / 32.0 };
static const double b4[] =
  { 1932.0 / 2197.0, -7200.0 / 2197.0, 7296.0 / 2197.0 };
static const double b5[] =
  { 8341.0 / 4104.0, -32832.0 / 4104.0, 29440.0 / 4104.0, -845.0 / 4104.0 };
static const double b6[] =
  { -6080.0 / 20520.0, 41040.0 / 20520.0, -28352.0 / 20520.0,
  9295.0 / 20520.0, -5643.0 / 20520.0
};

static const double c1 = 902880.0 / 7618050.0;
static const double c3 = 3953664.0 / 7618050.0;
static const double c4 = 3855735.0 / 7618050.0;
static const double c5 = -1371249.0 / 7618050.0;
static const double c6 = 277020.0 / 7618050.0;

/* These are the differences of fifth and fourth order coefficients
   for error estimation */

static const double ec[] = { 0.0,
  1.0 / 360.0,
  0.0,
  -128.0 / 4275.0,
  -2197.0 / 75240.0,
  1.0 / 50.0,
  2.0 / 55.0
};

typedef struct
{
  double *k1;
  double *k2;
  double *k3;
  double *k4;
  double *k5;
  double *k6;
  double *y0;
  double *ytmp;
}
rkf45_state_t;

static void *
rkf45_alloc (size_t dim)
{
  rkf45_state_t *state = (rkf45_state_t *) malloc (sizeof (rkf45_state_t));

  if (state == 0)
    {
      GSL_ERROR_NULL ("failed to allocate space for rkf45_state", GSL_ENOMEM);
    }

  state->k1 = (double *) malloc (dim * sizeof (double));

  if (state->k1 == 0)
    {
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for k1", GSL_ENOMEM);
    }

  state->k2 = (double *) malloc (dim * sizeof (double));

  if (state->k2 == 0)
    {
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for k2", GSL_ENOMEM);
    }

  state->k3 = (double *) malloc (dim * sizeof (double));

  if (state->k3 == 0)
    {
      free (state->k2);
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for k3", GSL_ENOMEM);
    }

  state->k4 = (double *) malloc (dim * sizeof (double));

  if (state->k4 == 0)
    {
      free (state->k3);
      free (state->k2);
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for k4", GSL_ENOMEM);
    }

  state->k5 = (double *) malloc (dim * sizeof (double));

  if (state->k5 == 0)
    {
      free (state->k4);
      free (state->k3);
      free (state->k2);
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for k5", GSL_ENOMEM);
    }

  state->k6 = (double *) malloc (dim * sizeof (double));

  if (state->k6 == 0)
    {
      free (state->k5);
      free (state->k4);
      free (state->k3);
      free (state->k2);
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for k6", GSL_ENOMEM);
    }

  state->y0 = (double *) malloc (dim * sizeof (double));

  if (state->y0 == 0)
    {
      free (state->k6);
      free (state->k5);
      free (state->k4);
      free (state->k3);
      free (state->k2);
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for y0", GSL_ENOMEM);
    }

  state->ytmp = (double *) malloc (dim * sizeof (double));

  if (state->ytmp == 0)
    {
      free (state->y0);
      free (state->k6);
      free (state->k5);
      free (state->k4);
      free (state->k3);
      free (state->k2);
      free (state->k1);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for ytmp", GSL_ENOMEM);
    }

  return state;
}


static int
rkf45_apply (void *vstate,
             size_t dim,
             double t,
             double h,
             double y[],
             double yerr[],
             const double dydt_in[],
             double dydt_out[], const gsl_odeiv2_system * sys)
{
  rkf45_state_t *state = (rkf45_state_t *) vstate;

  size_t i;

  double *const k1 = state->k1;
  double *const k2 = state->k2;
  double *const k3 = state->k3;
  double *const k4 = state->k4;
  double *const k5 = state->k5;
  double *const k6 = state->k6;
  double *const ytmp = state->ytmp;
  double *const y0 = state->y0;

  DBL_MEMCPY (y0, y, dim);

  /* k1 step */
  if (dydt_in != NULL)
    {
      DBL_MEMCPY (k1, dydt_in, dim);
    }
  else
    {
      int s = GSL_ODEIV_FN_EVAL (sys, t, y, k1);

      if (s != GSL_SUCCESS)
        {
          return s;
        }
    }

  for (i = 0; i < dim; i++)
    ytmp[i] = y[i] + ah[0] * h * k1[i];

  /* k2 step */
  {
    int s = GSL_ODEIV_FN_EVAL (sys, t + ah[0] * h, ytmp, k2);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  for (i = 0; i < dim; i++)
    ytmp[i] = y[i] + h * (b3[0] * k1[i] + b3[1] * k2[i]);

  /* k3 step */
  {
    int s = GSL_ODEIV_FN_EVAL (sys, t + ah[1] * h, ytmp, k3);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  for (i = 0; i < dim; i++)
    ytmp[i] = y[i] + h * (b4[0] * k1[i] + b4[1] * k2[i] + b4[2] * k3[i]);

  /* k4 step */
  {
    int s = GSL_ODEIV_FN_EVAL (sys, t + ah[2] * h, ytmp, k4);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  for (i = 0; i < dim; i++)
    ytmp[i] =
      y[i] + h * (b5[0] * k1[i] + b5[1] * k2[i] + b5[2] * k3[i] +
                  b5[3] * k4[i]);

  /* k5 step */
  {
    int s = GSL_ODEIV_FN_EVAL (sys, t + ah[3] * h, ytmp, k5);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  for (i = 0; i < dim; i++)
    ytmp[i] =
      y[i] + h * (b6[0] * k1[i] + b6[1] * k2[i] + b6[2] * k3[i] +
                  b6[3] * k4[i] + b6[4] * k5[i]);

  /* k6 step and final sum */
  {
    int s = GSL_ODEIV_FN_EVAL (sys, t + ah[4] * h, ytmp, k6);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  for (i = 0; i < dim; i++)
    {
      const double d_i =
        c1 * k1[i] + c3 * k3[i] + c4 * k4[i] + c5 * k5[i] + c6 * k6[i];
      y[i] += h * d_i;
    }

  /* Derivatives at output */

  if (dydt_out != NULL)
    {
      int s = GSL_ODEIV_FN_EVAL (sys, t + h, y, dydt_out);

      if (s != GSL_SUCCESS)
        {
          /* Restore initial values */
          DBL_MEMCPY (y, y0, dim);

          return s;
        }
    }

  /* difference between 4th and 5th order */
  for (i = 0; i < dim; i++)
    {
      yerr[i] = h * (ec[1] * k1[i] + ec[3] * k3[i] + ec[4] * k4[i]
                     + ec[5] * k5[i] + ec[6] * k6[i]);
    }

  return GSL_SUCCESS;
}


static int
rkf45_reset (void *vstate, size_t dim)
{
  rkf45_state_t *state = (rkf45_state_t *) vstate;

  DBL_ZERO_MEMSET (state->k1, dim);
  DBL_ZERO_MEMSET (state->k2, dim);
  DBL_ZERO_MEMSET (state->k3, dim);
  DBL_ZERO_MEMSET (state->k4, dim);
  DBL_ZERO_MEMSET (state->k5, dim);
  DBL_ZERO_MEMSET (state->k6, dim);
  DBL_ZERO_MEMSET (state->ytmp, dim);
  DBL_ZERO_MEMSET (state->y0, dim);

  return GSL_SUCCESS;
}

static unsigned int
rkf45_order (void *vstate)
{
  rkf45_state_t *state = (rkf45_state_t *) vstate;
  state = 0;                    /* prevent warnings about unused parameters */
  return 5;
}

static void
rkf45_free (void *vstate)
{
  rkf45_state_t *state = (rkf45_state_t *) vstate;

  free (state->ytmp);
  free (state->y0);
  free (state->k6);
  free (state->k5);
  free (state->k4);
  free (state->k3);
  free (state->k2);
  free (state->k1);
  free (state);
}

static const gsl_odeiv2_step_type rkf45_type = { "rkf45",       /* name */
  1,                            /* can use dydt_in */
  1,                            /* gives exact dydt_out */
  &rkf45_alloc,
  &rkf45_apply,
  &stepper_set_driver_null,
  &rkf45_reset,
  &rkf45_order,
  &rkf45_free
};

const gsl_odeiv2_step_type *gsl_odeiv2_step_rkf45 = &rkf45_type;
