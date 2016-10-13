/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'module_info.h' is part of ERT - Ensemble based Reservoir Tool.

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
#ifndef ERT_MODULE_INFO_H
#define ERT_MODULE_INFO_H

#include <ert/analysis/module_data_block_vector.h>
#include <ert/analysis/module_obs_block_vector.h>

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct module_info_struct module_info_type;

  module_info_type                * module_info_alloc(const char* ministep_name);
  void                              module_info_free();
  char                          *   module_info_get_ministep_name(const module_info_type * module_info);
  module_data_block_vector_type *   module_info_get_data_block_vector(const module_info_type * module_info);
  module_obs_block_vector_type  *   module_info_get_obs_block_vector(const module_info_type * module_info);

  UTIL_IS_INSTANCE_HEADER( module_info );

#ifdef __cplusplus
}
#endif
#endif
