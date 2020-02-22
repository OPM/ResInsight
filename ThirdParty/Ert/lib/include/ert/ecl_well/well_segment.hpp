/*
   Copyright (C) 2013  Equinor ASA, Norway.

   The file 'well_segment.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_WELL_SEGMENT_H
#define ERT_WELL_SEGMENT_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_rsthead.hpp>

#include <ert/ecl_well/well_conn_collection.hpp>
#include <ert/ecl_well/well_conn.hpp>
#include <ert/ecl_well/well_rseg_loader.hpp>




  typedef struct well_segment_struct well_segment_type;

  well_segment_type * well_segment_alloc_from_kw( const ecl_kw_type * iseg_kw , const well_rseg_loader_type * rseg_loader , const ecl_rsthead_type * header , int well_nr, int segment_index , int segment_id);
  well_segment_type * well_segment_alloc(int segment_id , int outlet_segment_id , int branch_id , const double * rseg_data);
  void                well_segment_free(well_segment_type * segment );
  void                well_segment_free__(void * arg);

  bool                well_segment_active( const well_segment_type * segment );
  bool                well_segment_main_stem( const well_segment_type * segment );
  bool                well_segment_nearest_wellhead( const well_segment_type * segment );

  int                 well_segment_get_link_count( const well_segment_type * segment );
  int                 well_segment_get_branch_id( const well_segment_type * segment );
  int                 well_segment_get_outlet_id( const well_segment_type * segment );
  int                 well_segment_get_id( const well_segment_type * segment );
  well_segment_type * well_segment_get_outlet( const well_segment_type * segment );
  bool                well_segment_link( well_segment_type * segment , well_segment_type * outlet_segment );
  void                well_segment_link_strict( well_segment_type * segment , well_segment_type * outlet_segment );
  bool                well_segment_has_grid_connections( const well_segment_type * segment , const char * grid_name);
  bool                well_segment_has_global_grid_connections( const well_segment_type * segment);
  bool                well_segment_add_connection( well_segment_type * segment , const char * grid_name , well_conn_type * conn);
  const well_conn_collection_type * well_segment_get_connections(const well_segment_type * segment , const char * grid_name );
  const well_conn_collection_type * well_segment_get_global_connections(const well_segment_type * segment );
  bool                well_segment_well_is_MSW(int well_nr , const ecl_kw_type * iwel_kw , const ecl_rsthead_type * rst_head);

  double              well_segment_get_depth( const well_segment_type * segment );
  double              well_segment_get_length( const well_segment_type * segment );
  double              well_segment_get_total_length( const well_segment_type * segment );
  double              well_segment_get_diameter( const well_segment_type * segment );

  UTIL_IS_INSTANCE_HEADER( well_segment );

#ifdef __cplusplus
}
#endif
#endif
