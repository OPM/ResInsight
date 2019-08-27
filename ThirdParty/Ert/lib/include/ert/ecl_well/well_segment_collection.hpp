/*
   Copyright (C) 2013  Equinor ASA, Norway.

                   The file 'well_segment_collection.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_WELL_SEGMENT_COLLECTION_H
#define ERT_WELL_SEGMENT_COLLECTION_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/ecl/ecl_kw.hpp>

#include <ert/ecl_well/well_segment.hpp>
#include <ert/ecl_well/well_conn_collection.hpp>
#include <ert/ecl_well/well_branch_collection.hpp>
#include <ert/ecl_well/well_rseg_loader.hpp>

  typedef struct well_segment_collection_struct well_segment_collection_type;

  well_segment_collection_type * well_segment_collection_alloc(void);
  void                           well_segment_collection_free(well_segment_collection_type * segment_collection );
  int                            well_segment_collection_get_size( const well_segment_collection_type * segment_collection );
  void                           well_segment_collection_add( well_segment_collection_type * segment_collection , well_segment_type * segment);
  bool                           well_segment_collection_has_segment( const well_segment_collection_type * segment_collection , int segment_id);
  well_segment_type            * well_segment_collection_get( const well_segment_collection_type * segment_collection , int segment_id);
  well_segment_type            * well_segment_collection_iget( const well_segment_collection_type * segment_collection , int index);
  int                            well_segment_collection_load_from_kw( well_segment_collection_type * segment_collection , int well_nr ,
                                                                       const ecl_kw_type * iwel_kw ,
                                                                       const ecl_kw_type * iseg_kw ,
                                                                       const well_rseg_loader_type * rseg_loader ,
                                                                       const ecl_rsthead_type * rst_head,
                                                                       bool load_segment_information , bool * is_MSW_well);

  void                           well_segment_collection_link(const  well_segment_collection_type * segment_collection);
  void                           well_segment_collection_add_connections(well_segment_collection_type * segment_collection ,
                                                                         const char * grid_name ,
                                                                         const well_conn_collection_type * connections);
  void                           well_segment_collection_add_branches( const well_segment_collection_type * segment_collection ,
                                                                       well_branch_collection_type * branches );

#ifdef __cplusplus
}
#endif
#endif
