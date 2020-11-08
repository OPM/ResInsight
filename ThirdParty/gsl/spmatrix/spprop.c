/* spprop.c
 * 
 * Copyright (C) 2014 Patrick Alken
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

#include <gsl/gsl_spmatrix.h>
#include <gsl/gsl_errno.h>

/*
gsl_spmatrix_equal()
  Return 1 if a = b, 0 otherwise
*/

int
gsl_spmatrix_equal(const gsl_spmatrix *a, const gsl_spmatrix *b)
{
  const size_t M = a->size1;
  const size_t N = a->size2;

  if (b->size1 != M || b->size2 != N)
    {
      GSL_ERROR_VAL("matrices must have same dimensions", GSL_EBADLEN, 0);
    }
  else if (a->sptype != b->sptype)
    {
      GSL_ERROR_VAL("trying to compare different sparse matrix types", GSL_EINVAL, 0);
    }
  else
    {
      const size_t nz = a->nz;
      size_t n;

      if (nz != b->nz)
        return 0; /* different number of non-zero elements */

      if (GSL_SPMATRIX_ISTRIPLET(a))
        {
          /*
           * triplet formats could be out of order but identical, so use
           * gsl_spmatrix_get() on b for each aij
           */
          for (n = 0; n < nz; ++n)
            {
              double bij = gsl_spmatrix_get(b, a->i[n], a->p[n]);

              if (a->data[n] != bij)
                return 0;
            }
        }
      else if (GSL_SPMATRIX_ISCCS(a))
        {
          /*
           * for CCS, both matrices should have everything
           * in the same order
           */

          /* check row indices and data */
          for (n = 0; n < nz; ++n)
            {
              if ((a->i[n] != b->i[n]) || (a->data[n] != b->data[n]))
                return 0;
            }

          /* check column pointers */
          for (n = 0; n < a->size2 + 1; ++n)
            {
              if (a->p[n] != b->p[n])
                return 0;
            }
        }
      else if (GSL_SPMATRIX_ISCRS(a))
        {
          /*
           * for CRS, both matrices should have everything
           * in the same order
           */

          /* check column indices and data */
          for (n = 0; n < nz; ++n)
            {
              if ((a->i[n] != b->i[n]) || (a->data[n] != b->data[n]))
                return 0;
            }

          /* check row pointers */
          for (n = 0; n < a->size1 + 1; ++n)
            {
              if (a->p[n] != b->p[n])
                return 0;
            }
        }
      else
        {
          GSL_ERROR_VAL("unknown sparse matrix type", GSL_EINVAL, 0);
        }

      return 1;
    }
} /* gsl_spmatrix_equal() */
