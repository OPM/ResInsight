/*
   Copyright (C) 2013  Equinor ASA, Norway.

   The file 'nnc_info.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_NNC_INFO_H
#define ERT_NNC_INFO_H

#include <ert/util/int_vector.hpp>
#include <ert/util/type_macros.hpp>

#include <ert/ecl/nnc_vector.hpp>

typedef struct nnc_info_struct nnc_info_type;

#ifdef __cplusplus
#include <vector>
  const std::vector<int>& nnc_info_get_grid_index_list(const nnc_info_type * nnc_info, int lgr_nr);
  const std::vector<int>& nnc_info_iget_grid_index_list(const nnc_info_type * nnc_info, int lgr_index);
  const std::vector<int>& nnc_info_get_self_grid_index_list(const nnc_info_type * nnc_info);
#endif

#ifdef __cplusplus
extern "C" {
#endif

  UTIL_IS_INSTANCE_HEADER(nnc_info);

  nnc_info_type         * nnc_info_alloc(int lgr_nr);
  void                    nnc_info_free( nnc_info_type * nnc_info );
  void                    nnc_info_add_nnc(nnc_info_type * nnc_info, int lgr_nr, int global_cell_number, int nnc_index);

  nnc_vector_type       * nnc_info_iget_vector( const nnc_info_type * nnc_info , int lgr_index);

  nnc_vector_type       * nnc_info_get_vector( const nnc_info_type * nnc_info , int lgr_nr);

  nnc_vector_type       * nnc_info_get_self_vector( const nnc_info_type * nnc_info );

  int                     nnc_info_get_lgr_nr(const nnc_info_type * nnc_info );
  int                     nnc_info_get_size( const nnc_info_type * nnc_info );
  int                     nnc_info_get_total_size( const nnc_info_type * nnc_info );
  void                    nnc_info_fprintf(const nnc_info_type * nnc_info , FILE * stream);

  bool                    nnc_info_equal( const nnc_info_type * nnc_info1 , const nnc_info_type * nnc_info2 );
  nnc_info_type *         nnc_info_alloc_copy( const nnc_info_type * src_info );
  bool                    nnc_info_has_grid_index_list( const nnc_info_type * nnc_info, int lgr_nr );

#ifdef __cplusplus
}
#endif
#endif

