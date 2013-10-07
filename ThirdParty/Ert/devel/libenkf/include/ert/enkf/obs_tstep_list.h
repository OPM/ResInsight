/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'obs_tstep_list.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __OBS_TSTEP_LIST_H__
#define __OBS_TSTEP_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/type_macros.h>

  typedef struct obs_tstep_list_struct obs_tstep_list_type;

  obs_tstep_list_type * obs_tstep_list_alloc();
  void                  obs_tstep_list_free( obs_tstep_list_type * list );
  bool                  obs_tstep_list_all_active( const obs_tstep_list_type * list );
  int                   obs_tstep_list_get_size( const obs_tstep_list_type * list );
  void                  obs_tstep_list_add_tstep( obs_tstep_list_type * list , int tstep);
  void                  obs_tstep_list_add_range( obs_tstep_list_type * list , int step1 , int step2);
  bool                  obs_tstep_list_contains( const obs_tstep_list_type * list , int tstep);
  int                   obs_tstep_list_iget( const obs_tstep_list_type * list , int index);
  int                   obs_tstep_list_get_last( const obs_tstep_list_type * list );

  UTIL_IS_INSTANCE_HEADER( obs_tstep_list );

#ifdef __cplusplus
}
#endif

#endif
