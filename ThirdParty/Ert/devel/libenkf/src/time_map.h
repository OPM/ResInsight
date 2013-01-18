/*
   Copyright (C) 2011  Statoil ASA, Norway. 
   The file 'time_map.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef __TIME_MAP_H__
#define __TIME_MAP_H__

#ifdef __cplusplus 
extern "C" {
#endif 

#include <time.h>


typedef struct time_map_struct time_map_type;


  time_map_type  * time_map_alloc( );
  void             time_map_free( time_map_type * map );
  void             time_map_update( time_map_type * map , int step , time_t time);
  void             time_map_summary_update( time_map_type * map , const ecl_sum_type * ecl_sum);
  time_t           time_map_iget( time_map_type * map , int step );
  void             time_map_fwrite( time_map_type * map , const char * filename);
  void             time_map_fread( time_map_type * map , const char * filename);
  double           time_map_iget_sim_days( time_map_type * map , int step );
  int              time_map_get_last_step( time_map_type * map);

#ifdef __cplusplus 
}
#endif
#endif
