/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'analysis_test_module_info.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>

#include <ert/analysis/module_info.h>


int main(int argc , char ** argv) {

  const char* ministep_name = "SOME MINISTEP";
  module_info_type * module_info = module_info_alloc( ministep_name );
  test_assert_true( module_info_is_instance( module_info ));

  module_data_block_vector_type * module_data_block_vector =  module_info_get_data_block_vector(module_info);
  test_assert_true( module_data_block_vector_is_instance( module_data_block_vector ));

  module_obs_block_vector_type * module_obs_block_vector =  module_info_get_obs_block_vector(module_info);
  test_assert_true( module_obs_block_vector_is_instance( module_obs_block_vector ));

  int index_list[1] = { 1 };
  module_data_block_type * module_data_block = module_data_block_alloc( "PARAMETER", &index_list[0],  0, 1 );
  test_assert_true( module_data_block_is_instance( module_data_block ));
  module_data_block_vector_add_data_block(module_data_block_vector, module_data_block);


  module_obs_block_type * module_obs_block = module_obs_block_alloc( "OBS", &index_list[0],  0, 1 );
  test_assert_true( module_obs_block_is_instance( module_obs_block ));
  module_obs_block_vector_add_obs_block(module_obs_block_vector, module_obs_block);

  module_info_free( module_info );
  exit(0);
}


