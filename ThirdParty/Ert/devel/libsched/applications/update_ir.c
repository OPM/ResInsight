/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'update_ir.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <signal.h>
#include <util.h>
#include <sched_file.h>
#include <history.h>
#include <sched_kw_wconinje.h>
#include <hash.h>
#include <ecl_util.h>
#include <parser.h>
#include <stringlist.h>

void scale_injectors(void * void_kw , int report_step , void * arg) {
  sched_kw_wconinje_type * kw = sched_kw_wconinje_safe_cast( void_kw );
  hash_type * well_hash = hash_safe_cast( arg );
  hash_iter_type * hash_iter = hash_iter_alloc( well_hash );
  

  while ( !hash_iter_is_complete(hash_iter) ) {
    const char * well   = hash_iter_get_next_key( hash_iter );
    double scale_factor = hash_get_double( well_hash , well );

    sched_kw_wconinje_scale_surface_flow(kw , well , scale_factor);
    printf("Scaling injector: %s/%g \n",well , scale_factor);
  }
  hash_iter_free( hash_iter );
}



static hash_type * parse_multir( const char * multir_file ) {
  parser_type  * parser = parser_alloc(" \n\t",NULL ,NULL ,  NULL , NULL , NULL);
  stringlist_type * tokens    = parser_tokenize_file ( parser , multir_file , true );
  hash_type * hash = hash_alloc();
  int i;

  for (i = 0; i < stringlist_get_size(tokens); i += 2) {
    const char * well      = stringlist_iget( tokens , i);
    const char * multir_st = stringlist_iget( tokens , i + 1); 
    double multir;

    if (util_sscanf_double( multir_st , &multir)) 
      hash_insert_double( hash , well , multir);
    else
      util_abort("%s: failed to parse: %s as double \n",__func__ , multir_st);
    
  }
  stringlist_free( tokens );
  parser_free( parser );
  return hash;
}


void install_SIGNALS(void) {
  signal(SIGSEGV , util_abort_signal);
  signal(SIGINT  , util_abort_signal);
  signal(SIGKILL , util_abort_signal);
}




int main(int argc, char **argv)
{
  if(argc < 4)
  {
    printf("Usage: sched_test.x data_file my_sched_file.SCH  update_ir.txt \n");
    return 0;
  }

  time_t start_time;
  int    num_restart_files;
  int    last_restart_file;
  char * data_file     = argv[1];
  char * schedule_file = argv[2];
  char * multir_file   = argv[3];

  hash_type * hash;
  sched_file_type * sched_file;

  start_time = ecl_util_get_start_date( data_file );
  sched_file = sched_file_parse_alloc( schedule_file , start_time);
  unlink( schedule_file );
  hash = parse_multir( multir_file );
  sched_file_update(sched_file , WCONINJE , scale_injectors , hash);
  sched_file_fprintf(sched_file , schedule_file );
  sched_file_free( sched_file );
  hash_free( hash );
  

  return 0;
}
