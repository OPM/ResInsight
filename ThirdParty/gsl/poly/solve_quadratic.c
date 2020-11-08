/* poly/solve_quadratic.c
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

/* solve_quadratic.c - finds the real roots of a x^2 + b x + c = 0 */

#include <config.h>
#include <math.h>

#include <gsl/gsl_poly.h>

int 
gsl_poly_solve_quadratic (double a, double b, double c, 
                          double *x0, double *x1)
{
  if (a == 0) /* Handle linear case */
    {
      if (b == 0)
        {
          return 0;
        }
      else
        {
          *x0 = -c / b;
          return 1;
        };
    }

  {
    double disc = b * b - 4 * a * c;
    
    if (disc > 0)
      {
        if (b == 0)
          {
            double r = sqrt (-c / a);
            *x0 = -r;
            *x1 =  r;
          }
        else
          {
            double sgnb = (b > 0 ? 1 : -1);
            double temp = -0.5 * (b + sgnb * sqrt (disc));
            double r1 = temp / a ;
            double r2 = c / temp ;
            
            if (r1 < r2) 
              {
                *x0 = r1 ;
                *x1 = r2 ;
              } 
            else 
              {
                *x0 = r2 ;
                  *x1 = r1 ;
              }
          }
        return 2;
      }
    else if (disc == 0) 
      {
        *x0 = -0.5 * b / a ;
        *x1 = -0.5 * b / a ;
        return 2 ;
      }
    else
      {
        return 0;
      }
  }
}

