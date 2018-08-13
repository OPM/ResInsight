/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'arg_pack.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ARG_PACK_H
#define ERT_ARG_PACK_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/node_ctype.hpp>
#include <ert/util/type_macros.hpp>

typedef struct arg_pack_struct arg_pack_type;
typedef void   (arg_node_free_ftype)  (void *);
typedef void * (arg_node_copyc_ftype) (const void *);

  arg_pack_type * arg_pack_alloc();
  UTIL_SAFE_CAST_HEADER( arg_pack );
  UTIL_SAFE_CAST_HEADER_CONST( arg_pack );
  UTIL_IS_INSTANCE_HEADER( arg_pack );

  void            arg_pack_free(arg_pack_type * );
  void            arg_pack_free__(void *);
  void            arg_pack_clear(arg_pack_type *);
  void            arg_pack_lock(arg_pack_type *);
  void            arg_pack_fscanf(arg_pack_type * arg , FILE * stream, const char * filename);
  void            arg_pack_fprintf(const arg_pack_type * , FILE * );

  void            arg_pack_append_ptr(arg_pack_type * , void *);
  void            arg_pack_append_const_ptr(arg_pack_type * , const void *);
  void            arg_pack_append_owned_ptr(arg_pack_type * , void * , arg_node_free_ftype *);
  void            arg_pack_append_copy(arg_pack_type * , void * , arg_node_copyc_ftype * , arg_node_free_ftype *);

  /*
    void            arg_pack_iset_copy(arg_pack_type * arg_pack , int index , void * ptr, arg_node_copyc_ftype * copyc , arg_node_free_ftype * freef);
    void            arg_pack_iset_ptr(arg_pack_type * arg_pack, int index , void * ptr);
    void            arg_pack_iset_owned_ptr(arg_pack_type * arg_pack, int index , void * ptr, arg_node_free_ftype * freef);
  */
  const void    * arg_pack_iget_const_ptr( const arg_pack_type * arg_pack , int index);
  void          * arg_pack_iget_ptr(const arg_pack_type * , int);
  void          * arg_pack_iget_adress(const arg_pack_type * , int);
  node_ctype      arg_pack_iget_ctype(const arg_pack_type * arg_pack ,int index);

  int arg_pack_size( const arg_pack_type * arg_pack );

  /*****************************************************************/

#define APPEND_TYPED_HEADER(type) void arg_pack_append_ ## type (arg_pack_type * , type);
#define IGET_TYPED_HEADER(type)   type arg_pack_iget_ ## type( const arg_pack_type * , int );
#define ISET_TYPED_HEADER(type)   void arg_pack_iset_ ## type( arg_pack_type * , int , type value);

APPEND_TYPED_HEADER(int)
APPEND_TYPED_HEADER(bool)
APPEND_TYPED_HEADER(char)
APPEND_TYPED_HEADER(float)
APPEND_TYPED_HEADER(double)
APPEND_TYPED_HEADER(size_t)

IGET_TYPED_HEADER(int)
IGET_TYPED_HEADER(bool)
IGET_TYPED_HEADER(char)
IGET_TYPED_HEADER(float)
IGET_TYPED_HEADER(double)
IGET_TYPED_HEADER(size_t)

ISET_TYPED_HEADER(int)
ISET_TYPED_HEADER(bool)
ISET_TYPED_HEADER(char)
ISET_TYPED_HEADER(float)
ISET_TYPED_HEADER(double)
ISET_TYPED_HEADER(size_t)

#undef APPEND_TYPED_HEADER
#undef GET_TYPED_HEADER


#ifdef __cplusplus
}
#endif
#endif

