/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'path_stack.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_PATH_STACK_H
#define ERT_PATH_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct path_stack_struct path_stack_type;

  path_stack_type * path_stack_alloc();
  void              path_stack_pop( path_stack_type * path_stack );
  void              path_stack_push_cwd( path_stack_type * path_stack );
  bool              path_stack_push( path_stack_type * path_stack , const char * path );
  void              path_stack_free( path_stack_type * path_stack );
  int               path_stack_size( const path_stack_type * path_stack );

#ifdef __cplusplus
}
#endif


#endif
