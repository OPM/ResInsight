/* cdf/flatinv.c
 * 
 * Copyright (C) 2003, 2007 Brian Gough
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
#include <gsl/gsl_cdf.h>

double
gsl_cdf_flat_Pinv (const double P, const double a, const double b)
{
  double x;

  if (P == 1.0)
    {
      return b;
    }
  else if (P == 0.0)
    {
      return a;
    }

  x = (1 - P) * a + P * b;

  return x;
}

double
gsl_cdf_flat_Qinv (const double Q, const double a, const double b)
{
  double x;

  if (Q == 0.0)
    {
      return b;
    }
  else if (Q == 1.0)
    {
      return a;
    }

  x = Q * a + (1 - Q) * b;

  return x;
}
