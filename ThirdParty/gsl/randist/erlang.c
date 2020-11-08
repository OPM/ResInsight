/* randist/erlang.c
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 James Theiler, Brian Gough
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
#include <math.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

/* The sum of N samples from an exponential distribution gives an
   Erlang distribution

   p(x) dx = x^(n-1) exp (-x/a) / ((n-1)!a^n) dx

   for x = 0 ... +infty */

double
gsl_ran_erlang (const gsl_rng * r, const double a, const double n)
{
  return gsl_ran_gamma (r, n, a);
}

double
gsl_ran_erlang_pdf (const double x, const double a, const double n)
{
  if (x <= 0) 
    {
      return 0 ;
    }
  else
    {
      double p;
      double lngamma = gsl_sf_lngamma (n);

      p = exp ((n - 1) * log (x/a) - x/a - lngamma) / a;
      return p;
    }
}
