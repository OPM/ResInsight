/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_history.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_HISTORY_H__
#define __SCHED_HISTORY_H__

#ifdef __cplusplus 
extern "C" {
#endif

#include <time.h>

#include <ert/util/double_vector.h>

#include <ert/sched/sched_file.h>

typedef struct sched_history_struct sched_history_type;


  time_t                sched_history_iget_time_t( const sched_history_type * sched_history , int restart_nr );
  void                  sched_history_update( sched_history_type * sched_history, const sched_file_type * sched_file );
  sched_history_type *  sched_history_alloc( const char * sep_string );
  void                  sched_history_free( sched_history_type * sched_history );
  double                sched_history_iget( const sched_history_type * sched_history , const char * key , int report_step);
  void                  sched_history_init_vector( const sched_history_type * sched_history , const char * key , double_vector_type * value);
  void                  sched_history_fprintf_group_structure( sched_history_type * sched_history , int report_step );
  const char          * sched_history_get_join_string( const sched_history_type * sched_history );
  void                  sched_history_fprintf_index_keys( const sched_history_type * sched_history , FILE * stream );
  bool                  sched_history_has_key( const sched_history_type * sched_history , const char * key);
  void                  sched_history_fprintf( const sched_history_type * sched_history , const stringlist_type * key_list , FILE * stream);
  bool                  sched_history_well_open( const sched_history_type * sched_history , const char * well_name , int report_step );
  bool                  sched_history_has_well( const sched_history_type * sched_history , const char * well_name);
  bool                  sched_history_has_group( const sched_history_type * sched_history , const char * group_name);
  bool                  sched_history_group_exists( const sched_history_type * sched_history , const char * group_name , int report_step );
  int                   sched_history_get_last_history( const sched_history_type * sched_history );
  bool                  sched_history_open( const sched_history_type * sched_history , const char * key , int report_step);
  
#ifdef __cplusplus 
}
#endif

#endif
