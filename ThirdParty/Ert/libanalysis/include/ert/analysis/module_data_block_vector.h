/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_data_block_vector.h' is part of ERT - Ensemble based Reservoir Tool.

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
#ifndef ERT_MODULE_DATA_BLOCK_VECTOR_H
#define ERT_MODULE_DATA_BLOCK_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif



#include <ert/analysis/module_data_block.h>

  typedef struct module_data_block_vector_struct module_data_block_vector_type;

  module_data_block_vector_type   * module_data_block_vector_alloc();
  void                              module_data_block_vector_free();
  void                              module_data_block_vector_add_data_block( module_data_block_vector_type * module_data_block_vector , const module_data_block_type * data_block);
  module_data_block_type          * module_data_block_vector_iget_module_data_block(const module_data_block_vector_type * module_data_block_vector, int index);
  int                               module_data_block_vector_get_size(const module_data_block_vector_type * module_data_block_vector);

  UTIL_IS_INSTANCE_HEADER( module_data_block_vector );

#ifdef __cplusplus
}
#endif
#endif
