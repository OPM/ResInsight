/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'well_info.c' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_WELL_INFO_H
#define ERT_WELL_INFO_H



#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_grid.hpp>

#include <ert/ecl_well/well_ts.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct well_info_struct well_info_type;

  well_info_type *  well_info_alloc(const ecl_grid_type * grid);
  void              well_info_add_UNRST_wells2( well_info_type * well_info , ecl_file_view_type * rst_view, bool load_segment_information);
  void              well_info_add_UNRST_wells( well_info_type * well_info , ecl_file_type * rst_file, bool load_segment_information);
  void              well_info_add_wells( well_info_type * well_info , ecl_file_type * rst_file , int report_nr , bool load_segment_information);
  void              well_info_add_wells2( well_info_type * well_info , ecl_file_view_type * rst_view , int report_nr, bool load_segment_information);
  void              well_info_load_rstfile( well_info_type * well_info , const char * filename, bool load_segment_information);
  void              well_info_load_rst_eclfile( well_info_type * well_info , ecl_file_type * rst_file , bool load_segment_information);
  void              well_info_free( well_info_type * well_info );

  well_ts_type    * well_info_get_ts( const well_info_type * well_info , const char *well_name);
  int               well_info_get_num_wells( const well_info_type * well_info );
  const char      * well_info_iget_well_name( const well_info_type * well_info, int well_index);
  bool              well_info_has_well( well_info_type * well_info , const char * well_name );

  well_state_type * well_info_get_state_from_time( const well_info_type * well_info , const char * well_name , time_t sim_time);
  well_state_type * well_info_get_state_from_report( const well_info_type * well_info , const char * well_name , int report_step );
  well_state_type * well_info_iget_state( const well_info_type * well_info , const char * well_name , int time_index);
  well_state_type * well_info_iiget_state( const well_info_type * well_info , int well_index , int time_index);

#ifdef __cplusplus
}
#endif

#endif
