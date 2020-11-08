/* rng/rng.c
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_rng.h>

gsl_rng *
gsl_rng_alloc (const gsl_rng_type * T)
{

  gsl_rng *r = (gsl_rng *) malloc (sizeof (gsl_rng));

  if (r == 0)
    {
      GSL_ERROR_VAL ("failed to allocate space for rng struct",
                        GSL_ENOMEM, 0);
    };

  r->state = calloc (1, T->size);

  if (r->state == 0)
    {
      free (r);         /* exception in constructor, avoid memory leak */

      GSL_ERROR_VAL ("failed to allocate space for rng state",
                        GSL_ENOMEM, 0);
    };

  r->type = T;

  gsl_rng_set (r, gsl_rng_default_seed);        /* seed the generator */

  return r;
}

int
gsl_rng_memcpy (gsl_rng * dest, const gsl_rng * src)
{
  if (dest->type != src->type)
    {
      GSL_ERROR ("generators must be of the same type", GSL_EINVAL);
    }

  memcpy (dest->state, src->state, src->type->size);

  return GSL_SUCCESS;
}

gsl_rng *
gsl_rng_clone (const gsl_rng * q)
{
  gsl_rng *r = (gsl_rng *) malloc (sizeof (gsl_rng));

  if (r == 0)
    {
      GSL_ERROR_VAL ("failed to allocate space for rng struct",
                        GSL_ENOMEM, 0);
    };

  r->state = malloc (q->type->size);

  if (r->state == 0)
    {
      free (r);         /* exception in constructor, avoid memory leak */

      GSL_ERROR_VAL ("failed to allocate space for rng state",
                        GSL_ENOMEM, 0);
    };

  r->type = q->type;

  memcpy (r->state, q->state, q->type->size);

  return r;
}

void
gsl_rng_set (const gsl_rng * r, unsigned long int seed)
{
  (r->type->set) (r->state, seed);
}

unsigned long int
gsl_rng_max (const gsl_rng * r)
{
  return r->type->max;
}

unsigned long int
gsl_rng_min (const gsl_rng * r)
{
  return r->type->min;
}

const char *
gsl_rng_name (const gsl_rng * r)
{
  return r->type->name;
}

size_t
gsl_rng_size (const gsl_rng * r)
{
  return r->type->size;
}

void *
gsl_rng_state (const gsl_rng * r)
{
  return r->state;
}

void
gsl_rng_print_state (const gsl_rng * r)
{
  size_t i;
  unsigned char *p = (unsigned char *) (r->state);
  const size_t n = r->type->size;

  for (i = 0; i < n; i++)
    {
      /* FIXME: we're assuming that a char is 8 bits */
      printf ("%.2x", *(p + i));
    }

}

void
gsl_rng_free (gsl_rng * r)
{
  RETURN_IF_NULL (r);
  free (r->state);
  free (r);
}
