/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'active_list.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ACTIVE_LIST_H__
#define __ACTIVE_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>


typedef struct active_list_struct active_list_type;

  active_list_type * active_list_alloc( ); 
  void               active_list_reset(active_list_type * );
  void               active_list_add_index(active_list_type * , int);
  void               active_list_free( active_list_type *);
  const int        * active_list_get_active(const active_list_type * );
  int                active_list_get_active_size(const active_list_type * , int total_size );
  void               active_list_set_all_active(active_list_type * );
  void               active_list_set_data_size(active_list_type *  , int );
  void               active_list_free( active_list_type * );
  active_mode_type   active_list_get_mode(const active_list_type * );
  void               active_list_free__( void * arg );
  active_list_type * active_list_alloc_copy( const active_list_type * src);
  void               active_list_fprintf( const active_list_type * active_list , bool obs , const char * key , FILE * stream );
  bool               active_list_iget( const active_list_type * active_list , int index );
  bool               active_list_equal( const active_list_type * active_list1 , const active_list_type * active_list2);
  void               active_list_copy( active_list_type * target , const active_list_type * src);  

UTIL_IS_INSTANCE_HEADER( active_list );

#ifdef __cplusplus
}
#endif
#endif
