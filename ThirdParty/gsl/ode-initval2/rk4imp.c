/* ode-initval2/rk4imp.c
 * 
 * Copyright (C) 2009, 2010 Tuomo Keskitalo
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

/* Based on rk4imp.c by Gerard Jungman */

/* Gaussian implicit 4th order Runge-Kutta method. Error estimation
   is carried out by the step doubling method.
 */

/* References: 

   Ascher, U.M., Petzold, L.R., Computer methods for ordinary
   differential and differential-algebraic equations, SIAM, 
   Philadelphia, 1998. ISBN 0898714125, 9780898714128
*/

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv2.h>

#include "odeiv_util.h"
#include "rksubs.c"
#include "modnewton1.c"

/* Stage of method */
#define RK4IMP_STAGE 2

typedef struct
{
  gsl_matrix *A;                /* Runge-Kutta coefficients */
  double *y_onestep;            /* Result with one step */
  double *y_twostep;            /* Result with two half steps */
  double *ytmp;                 /* Temporary work space */
  double *y_save;               /* Backup space */
  double *YZ;                   /* Runge-Kutta points */
  double *fYZ;                  /* Derivatives at YZ */
  gsl_matrix *dfdy;             /* Jacobian matrix */
  double *dfdt;                 /* time derivative of f */
  modnewton1_state_t *esol;     /* nonlinear equation solver */
  double *errlev;               /* desired error level of y */
  const gsl_odeiv2_driver *driver;      /* pointer to driver object */
}
rk4imp_state_t;

static void *
rk4imp_alloc (size_t dim)
{
  rk4imp_state_t *state = (rk4imp_state_t *) malloc (sizeof (rk4imp_state_t));

  if (state == 0)
    {
      GSL_ERROR_NULL ("failed to allocate space for rk4imp_state",
                      GSL_ENOMEM);
    }

  state->A = gsl_matrix_alloc (RK4IMP_STAGE, RK4IMP_STAGE);

  if (state->A == 0)
    {
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for A", GSL_ENOMEM);
    }

  state->y_onestep = (double *) malloc (dim * sizeof (double));

  if (state->y_onestep == 0)
    {
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for y_onestep", GSL_ENOMEM);
    }

  state->y_twostep = (double *) malloc (dim * sizeof (double));

  if (state->y_twostep == 0)
    {
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for y_onestep", GSL_ENOMEM);
    }

  state->ytmp = (double *) malloc (dim * sizeof (double));

  if (state->ytmp == 0)
    {
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for ytmp", GSL_ENOMEM);
    }

  state->y_save = (double *) malloc (dim * sizeof (double));

  if (state->y_save == 0)
    {
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for y_save", GSL_ENOMEM);
    }

  state->YZ = (double *) malloc (dim * RK4IMP_STAGE * sizeof (double));

  if (state->YZ == 0)
    {
      free (state->y_save);
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for YZ", GSL_ENOMEM);
    }

  state->fYZ = (double *) malloc (dim * RK4IMP_STAGE * sizeof (double));

  if (state->fYZ == 0)
    {
      free (state->YZ);
      free (state->y_save);
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for fYZ", GSL_ENOMEM);
    }

  state->dfdt = (double *) malloc (dim * sizeof (double));

  if (state->dfdt == 0)
    {
      free (state->fYZ);
      free (state->YZ);
      free (state->y_save);
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for dfdt", GSL_ENOMEM);
    }

  state->dfdy = gsl_matrix_alloc (dim, dim);

  if (state->dfdy == 0)
    {
      free (state->dfdt);
      free (state->fYZ);
      free (state->YZ);
      free (state->y_save);
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for dfdy", GSL_ENOMEM);
    }

  state->esol = modnewton1_alloc (dim, RK4IMP_STAGE);

  if (state->esol == 0)
    {
      gsl_matrix_free (state->dfdy);
      free (state->dfdt);
      free (state->fYZ);
      free (state->YZ);
      free (state->y_save);
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for esol", GSL_ENOMEM);
    }

  state->errlev = (double *) malloc (dim * sizeof (double));

  if (state->errlev == 0)
    {
      modnewton1_free (state->esol);
      gsl_matrix_free (state->dfdy);
      free (state->dfdt);
      free (state->fYZ);
      free (state->YZ);
      free (state->y_save);
      free (state->ytmp);
      free (state->y_twostep);
      free (state->y_onestep);
      gsl_matrix_free (state->A);
      free (state);
      GSL_ERROR_NULL ("failed to allocate space for errlev", GSL_ENOMEM);
    }

  state->driver = NULL;

  return state;
}

static int
rk4imp_apply (void *vstate, size_t dim, double t, double h,
              double y[], double yerr[],
              const double dydt_in[], double dydt_out[],
              const gsl_odeiv2_system * sys)
{
  /* Makes a Gaussian implicit 4th order Runge-Kutta with step size h
     and estimates the local error of the step.
   */

  rk4imp_state_t *state = (rk4imp_state_t *) vstate;

  double *const y_onestep = state->y_onestep;
  double *const y_twostep = state->y_twostep;
  double *const ytmp = state->ytmp;
  double *const y_save = state->y_save;
  double *const YZ = state->YZ; /* Runge-Kutta points */
  double *const fYZ = state->fYZ;
  gsl_matrix *const dfdy = state->dfdy;
  double *const dfdt = state->dfdt;
  double *const errlev = state->errlev;

  const modnewton1_state_t *esol = state->esol;

  /* Runge-Kutta coefficients */

  gsl_matrix *A = state->A;

  const double b[] = { 0.5, 0.5 };
  const double c[] = { (3.0 - M_SQRT3) / 6.0, (3.0 + M_SQRT3) / 6.0 };

  gsl_matrix_set (A, 0, 0, 1.0 / 4);
  gsl_matrix_set (A, 0, 1, (3 - 2 * sqrt (3)) / 12);
  gsl_matrix_set (A, 1, 0, (3 + 2 * sqrt (3)) / 12);
  gsl_matrix_set (A, 1, 1, 1.0 / 4);

  if (esol == NULL)
    {
      GSL_ERROR ("no non-linear equation solver speficied", GSL_EINVAL);
    }

  /* Get desired error levels via gsl_odeiv2_control object through driver
     object, which is a requirement for this stepper.
   */

  if (state->driver == NULL)
    {
      return GSL_EFAULT;
    }
  else
    {
      size_t i;

      for (i = 0; i < dim; i++)
        {
          if (dydt_in != NULL)
            {
              gsl_odeiv2_control_errlevel (state->driver->c, y[i],
                                           dydt_in[i], h, i, &errlev[i]);
            }
          else
            {
              gsl_odeiv2_control_errlevel (state->driver->c, y[i],
                                           0.0, h, i, &errlev[i]);
            }
        }
    }

  /* Evaluate Jacobian for modnewton1 */

  {
    int s = GSL_ODEIV_JA_EVAL (sys, t, y, dfdy->data, dfdt);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  /* Calculate a single step with size h */

  {
    int s = modnewton1_init ((void *) esol, A, h, dfdy, sys);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  {
    int s = modnewton1_solve ((void *) esol, A, c, t, h, y,
                              sys, YZ, errlev);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  {
    size_t j;

    for (j = 0; j < RK4IMP_STAGE; j++)
      {
        int s =
          GSL_ODEIV_FN_EVAL (sys, t + c[j] * h, &YZ[j * dim], &fYZ[j * dim]);

        if (s != GSL_SUCCESS)
          {
            return s;
          }
      }
  }

  {
    int s = rksubs (y_onestep, h, y, fYZ, b, RK4IMP_STAGE, dim);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  /* Error estimation by step doubling */

  {
    int s = modnewton1_init ((void *) esol, A, h / 2.0, dfdy, sys);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  /* 1st half step */

  {
    int s = modnewton1_solve ((void *) esol, A, c, t, h / 2.0, y,
                              sys, YZ, errlev);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  {
    size_t j;

    for (j = 0; j < RK4IMP_STAGE; j++)
      {
        int s = GSL_ODEIV_FN_EVAL (sys, t + c[j] * h / 2.0, &YZ[j * dim],
                                   &fYZ[j * dim]);
        if (s != GSL_SUCCESS)
          {
            return s;
          }
      }
  }

  {
    int s = rksubs (ytmp, h / 2.0, y, fYZ, b, RK4IMP_STAGE, dim);

    if (s != GSL_SUCCESS)
      return s;
  }

  /* Save original y values in case of error */

  DBL_MEMCPY (y_save, y, dim);

  /* 2nd half step */

  {
    int s = modnewton1_solve ((void *) esol, A, c, t + h / 2.0, h / 2.0,
                              ytmp, sys, YZ, errlev);

    if (s != GSL_SUCCESS)
      {
        return s;
      }
  }

  {
    size_t j;

    for (j = 0; j < RK4IMP_STAGE; j++)
      {
        int s =
          GSL_ODEIV_FN_EVAL (sys, t + h / 2.0 + c[j] * h / 2.0, &YZ[j * dim],
                             &fYZ[j * dim]);
        if (s != GSL_SUCCESS)
          {
            return s;
          }
      }
  }

  {
    int s = rksubs (y_twostep, h / 2.0, ytmp, fYZ, b, RK4IMP_STAGE, dim);

    if (s != GSL_SUCCESS)
      {
        DBL_MEMCPY (y, y_save, dim);
        return s;
      }
  }

  /* Note: rk4imp returns y using the results from two half steps
     instead of the single step since the results are freely
     available and more precise.
   */

  DBL_MEMCPY (y, y_twostep, dim);

  /* Error estimation */

  {
    size_t i;
    for (i = 0; i < dim; i++)
      {
        yerr[i] = ODEIV_ERR_SAFETY * 0.5 *
          fabs (y_twostep[i] - y_onestep[i]) / 15.0;
      }
  }

  /* Derivatives at output */

  if (dydt_out != NULL)
    {
      int s = GSL_ODEIV_FN_EVAL (sys, t + h, y, dydt_out);

      if (s != GSL_SUCCESS)
        {
          /* Restore original values */
          DBL_MEMCPY (y, y_save, dim);

          return s;
        }
    }

  return GSL_SUCCESS;
}

static int
rk4imp_set_driver (void *vstate, const gsl_odeiv2_driver * d)
{
  rk4imp_state_t *state = (rk4imp_state_t *) vstate;

  state->driver = d;

  return GSL_SUCCESS;
}

static int
rk4imp_reset (void *vstate, size_t dim)
{
  rk4imp_state_t *state = (rk4imp_state_t *) vstate;

  DBL_ZERO_MEMSET (state->y_onestep, dim);
  DBL_ZERO_MEMSET (state->y_twostep, dim);
  DBL_ZERO_MEMSET (state->ytmp, dim);
  DBL_ZERO_MEMSET (state->y_save, dim);
  DBL_ZERO_MEMSET (state->YZ, dim * RK4IMP_STAGE);
  DBL_ZERO_MEMSET (state->fYZ, dim * RK4IMP_STAGE);

  return GSL_SUCCESS;
}

static unsigned int
rk4imp_order (void *vstate)
{
  rk4imp_state_t *state = (rk4imp_state_t *) vstate;
  state = 0;                    /* prevent warnings about unused parameters */
  return 4;
}

static void
rk4imp_free (void *vstate)
{
  rk4imp_state_t *state = (rk4imp_state_t *) vstate;

  free (state->errlev);
  modnewton1_free (state->esol);
  gsl_matrix_free (state->dfdy);
  free (state->dfdt);
  free (state->fYZ);
  free (state->YZ);
  free (state->y_save);
  free (state->ytmp);
  free (state->y_twostep);
  free (state->y_onestep);
  gsl_matrix_free (state->A);
  free (state);
}

static const gsl_odeiv2_step_type rk4imp_type = {
  "rk4imp",                     /* name */
  1,                            /* can use dydt_in? */
  1,                            /* gives exact dydt_out? */
  &rk4imp_alloc,
  &rk4imp_apply,
  &rk4imp_set_driver,
  &rk4imp_reset,
  &rk4imp_order,
  &rk4imp_free
};

const gsl_odeiv2_step_type *gsl_odeiv2_step_rk4imp = &rk4imp_type;
