/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'rng.h' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#ifndef ERT_RNG_H
#define ERT_RNG_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

#include <ert/util/type_macros.hpp>


typedef enum {
  INIT_DEFAULT      = 0,   /* The rng is initialized with the default seed values. */
  INIT_CLOCK        = 1,   /* Four random seeds are calculated with the util_clock_seed() function. */
  INIT_DEV_RANDOM   = 2,   /* Random content is read with the function util_fread_dev_random(). */
  INIT_DEV_URANDOM  = 3    /* Random content is read with the function util_fread_dev_urandom(). */
} rng_init_mode;


typedef enum {
  MZRAN = 1
} rng_alg_type;


  typedef unsigned int ( rng_forward_ftype )        ( void * );
  typedef void         ( rng_set_state_ftype )      ( void * , const char * );
  typedef void         ( rng_get_state_ftype )      ( void * , char * );
  typedef void *       ( rng_alloc_ftype )          ( void );
  typedef void         ( rng_free_ftype )           ( void * );
  typedef void         ( rng_fscanf_ftype )         ( void * , FILE * );
  typedef void         ( rng_fprintf_ftype )        ( const void * , FILE * );

  typedef struct rng_struct rng_type;

  rng_type      * rng_alloc( rng_alg_type type , rng_init_mode init_mode );
  void            rng_free( rng_type * rng);
  void            rng_free( rng_type * rng);
  unsigned int    rng_forward( rng_type * rng );
  double          rng_get_double( rng_type * rng);
  void            rng_rng_init( rng_type * rng , rng_type * seed_src);
  void            rng_init( rng_type * rng , rng_init_mode init_mode );
  rng_alg_type    rng_get_type( const rng_type * rng );
  void            rng_fprintf_state( rng_type * rng , FILE * stream );
  void            rng_fscanf_state( rng_type * rng , FILE * stream );
  int             rng_state_size( const rng_type * rng );
  void            rng_save_state( rng_type * rng , const char * filename);
  void            rng_load_state( rng_type * rng , const char * filename);

  void            rng_set_state( rng_type * rng , const char * state);
  void            rng_get_state( const rng_type * rng , char * state);

  unsigned int    rng_forward( rng_type * rng );
  double          rng_get_double( rng_type * rng );
  int             rng_get_int( rng_type * rng , int max_value );
  unsigned int    rng_get_max_int(const rng_type * rng);

  double          rng_std_normal( rng_type * rng );
  void            rng_shuffle_int( rng_type * rng , int * data , size_t num_elements);
  void            rng_shuffle( rng_type * rng , char * data , size_t element_size , size_t num_elements);
  void            rng_free__( void * arg);

  UTIL_SAFE_CAST_HEADER( rng );

#ifdef __cplusplus
}
#endif
#endif
