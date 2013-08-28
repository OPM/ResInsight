/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   
   The file 'well_branch_collection.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __WELL_BRANCH_COLLECTION_H__
#define __WELL_BRANCH_COLLECTION_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/type_macros.h>

#include <ert/ecl_well/well_segment.h>

  typedef struct well_branch_collection_struct well_branch_collection_type;

  well_branch_collection_type * well_branch_collection_alloc();
  void                          well_branch_collection_free( well_branch_collection_type * branches );
  void                          well_branch_collection_free__( void * arg );
  bool                          well_branch_collection_has_branch( const well_branch_collection_type * branches , int branch_id);
  int                           well_branch_collection_get_size( const well_branch_collection_type * branches );
  const well_segment_type     * well_branch_collection_iget_start_segment( const well_branch_collection_type * branches , int index );
  const well_segment_type     * well_branch_collection_get_start_segment( const well_branch_collection_type * branches , int branch_id);
  bool                          well_branch_collection_add_start_segment( well_branch_collection_type * branches , const well_segment_type * start_segment);

  UTIL_IS_INSTANCE_HEADER( well_branch_collection );

#ifdef __cplusplus
}
#endif
#endif
