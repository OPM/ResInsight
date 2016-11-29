/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_obs_blocks.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef MODULE_OBS_BLOCKS_H
#define MODULE_OBS_BLOCKS_H

#include <ert/util/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct module_obs_block_struct {
   UTIL_TYPE_ID_DECLARATION;
   char                    * key;
   const int               * index_list;
   int                       D_row_start;
   int                       n_active;
  };

  typedef struct module_obs_block_struct module_obs_block_type;

  module_obs_block_type  * module_obs_block_alloc( const char * key, const int * index_list, const int row_start, const int n_active);
  const char *             module_obs_block_get_key(const module_obs_block_type * module_obs_block);
  const int                module_obs_block_get_row_start(const module_obs_block_type * module_obs_block);
  const int                module_obs_block_get_row_end(const module_obs_block_type * module_obs_block);
  const int  *             module_obs_block_get_active_indices(const module_obs_block_type * module_obs_block );
  void                     module_obs_block_free(module_obs_block_type * module_obs_block);
  void                     module_obs_block_free__( void * arg );

  UTIL_IS_INSTANCE_HEADER( module_obs_block );

#ifdef __cplusplus
}
#endif
#endif

