/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_history.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __WELL_HISTORY__
#define __WELL_HISTORY__

#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/size_t_vector.h>

#include <ert/sched/sched_kw.h>
#include <ert/sched/sched_kw_wconhist.h>
#include <ert/sched/well_index.h>
#include <ert/sched/group_history.h>

typedef struct well_history_struct  well_history_type;




  bool                  well_history_is_producer( const well_history_type * well_history , int report_step );
  wconhist_state_type * well_history_get_wconhist( well_history_type * well_history );
  well_history_type   * well_history_alloc( const char * well_name , const time_t_vector_type * time);
  void                  well_history_free__(void * arg);
  void                  well_history_add_keyword( well_history_type * well_history, const sched_kw_type * sched_kw , int  report_step );
  const void          * well_history_get_state_ptr( const well_history_type * well_history , sched_kw_type_enum kw_type );
  const char          * well_history_get_name( const well_history_type * well_history );
  
  sched_kw_type_enum    well_history_iget_active_kw( const well_history_type * history , int report_step );
  double                well_history_iget( well_index_type * index , int report_step );
  void                  well_history_set_parent( well_history_type * child_well , int report_step , const group_history_type * parent_group);
  group_history_type  * well_history_get_parent( well_history_type * child_well , int report_step );
  
  bool                  well_history_well_open( const well_history_type * well_history , int report_step );
  double                well_history_iget_WGPRH( const well_history_type * well_history , int report_step );
  double                well_history_iget_WOPRH( const well_history_type * well_history , int report_step );
  double                well_history_iget_WWPRH( const well_history_type * well_history , int report_step );
  
  UTIL_IS_INSTANCE_HEADER( well_history );


#ifdef __cplusplus
}
#endif

#endif
