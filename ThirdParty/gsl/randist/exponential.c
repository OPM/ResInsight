/* randist/exponential.c
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007, 2009 James Theiler, Brian Gough
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
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

/* The exponential distribution has the form

   p(x) dx = exp(-x/mu) dx/mu

   for x = 0 ... +infty */

double
gsl_ran_exponential (const gsl_rng * r, const double mu)
{
  double u = gsl_rng_uniform (r);

  return -mu * log1p (-u);
}

double
gsl_ran_exponential_pdf (const double x, const double mu)
{
  if (x < 0)
    {
      return 0 ;
    }
  else
    {
      double p = exp (-x/mu)/mu;
      
      return p;
    }
}
