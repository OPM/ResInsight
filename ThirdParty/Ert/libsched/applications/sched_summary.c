/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_summary.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>

#include <ert/sched/sched_file.h>
#include <ert/sched/sched_history.h>







int main (int argc , char ** argv) {
  time_t start_time;
  if (argc < 3) 
    util_exit("usage: ECLIPSE.DATA SCHEDULE_FILE   <key1>   <key2>   <key3>  ...\n");
  {
    const char * data_file       = argv[1];
    const char * schedule_file   = argv[2];
    start_time = ecl_util_get_start_date( data_file );
    {
      sched_history_type * sched_history = sched_history_alloc( ":" );
      sched_file_type * sched_file = sched_file_alloc(start_time);
      sched_file_parse(sched_file , schedule_file);
      sched_history_update( sched_history , sched_file );

      {
        stringlist_type * key_list = stringlist_alloc_new();
        
        for (int iarg=3; iarg < argc; iarg++) {
          if( sched_history_has_key( sched_history , argv[iarg] ))
            stringlist_append_ref( key_list , argv[iarg]);
          else
            fprintf(stderr,"** Warning the SCHEDULE file does not contain the key: %s \n",argv[iarg]);
        }
        
        sched_history_fprintf( sched_history , key_list , stdout );
        stringlist_free( key_list ) ;
      }
    }
  }
}
