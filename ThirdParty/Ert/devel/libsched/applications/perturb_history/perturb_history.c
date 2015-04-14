/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'perturb_history.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <sched_file.h>
#include <sched_history.h>
#include <sched_kw.h>
#include <sched_kw_wconhist.h>
#include <ecl_util.h>
#include <util.h>
#include <stdlib.h>
#include <stdio.h>
#include <time_t_vector.h>
#include <double_vector.h>
#include <path_fmt.h>
#include <math.h>
#include <pert_util.h>
#include <well_rate.h>
#include <group_rate.h>
#include <config.h>
#include <sched_types.h>
#include <msg.h>

/*****************************************************************/


void perturb_wconhist( void * void_kw , int restart_nr , void * arg) {
  sched_kw_wconhist_type * kw = sched_kw_wconhist_safe_cast( void_kw );
  {
    hash_type * group_hash = hash_safe_cast( arg );
    hash_iter_type * group_iter = hash_iter_alloc( group_hash );
    while (!hash_iter_is_complete( group_iter )) {
      group_rate_type * group_rate = hash_iter_get_next_value( group_iter );
      if (group_rate_is_producer( group_rate ))
        group_rate_update_wconhist( group_rate , kw , restart_nr );
    }
  }
}



void perturb_wconinje( void * void_kw , int restart_nr , void * arg) {
  sched_kw_wconinje_type * kw = sched_kw_wconinje_safe_cast( void_kw );
  {
    hash_type * group_hash = hash_safe_cast( arg );
    hash_iter_type * group_iter = hash_iter_alloc( group_hash );
    while (!hash_iter_is_complete( group_iter )) {
      group_rate_type * group_rate = hash_iter_get_next_value( group_iter );
      if (!group_rate_is_producer( group_rate ))
        group_rate_update_wconinje( group_rate , kw , restart_nr );
    }
  }
}




void config_init(config_parser_type * config ) {
  config_item_type * item;

  config_add_key_value(config , "NUM_REALIZATIONS" , true , CONFIG_INT );
  config_add_key_value(config , "SCHEDULE_FILE"    , true , CONFIG_EXISTING_FILE);
  config_add_key_value(config , "DATA_FILE"        , true , CONFIG_EXISTING_FILE);
  config_add_key_value(config , "TARGET"           , true , CONFIG_STRING );
  


  item = config_add_item( config , "GROUP_RATE" , false , true );  /* Group name as part of parsing */
  config_item_set_argc_minmax(item , 4 , 4 , 4 , (const config_item_types[4]) {   CONFIG_STRING,           /* Group name */
                                                                              CONFIG_STRING ,          /* Phase */
                                                                              CONFIG_STRING ,          /* PRODUCER / INJECTOR */
                                                                              CONFIG_EXISTING_FILE});  /* File with min / max shift */
  config_item_set_indexed_selection_set( item , 1 , 3 , (const char *[3]) { "OIL" , "GAS" , "WATER"});
  config_item_set_indexed_selection_set( item , 2 , 2 , (const char *[2]) { "PRODUCER" , "INJECTOR"});



  item = config_add_item( config , "WELL_RATE" , false , true );  /* Group name as part of parsing */
  config_item_set_argc_minmax(item , 4 , 4 , 4 , (const config_item_types[4])  {  CONFIG_STRING,         /* GROUP NAME */   
                                                                                  CONFIG_STRING ,        /* Well name */
                                                                                  CONFIG_FLOAT  ,        /* Corr_length (days) */
                                                                                  CONFIG_EXISTING_FILE});/* File with mean , std shift */
}



void load_groups( const config_parser_type * config , const sched_file_type * sched_file , hash_type * group_rates , const sched_history_type * sched_history , const time_t_vector_type * time_vector ) {
  int i;
  for (i=0; i < config_get_occurences( config , "GROUP_RATE" ); i++) {
    const char * group_name   = config_iget( config , "GROUP_RATE" , i , 0 );
    const char * phase_string = config_iget( config , "GROUP_RATE" , i , 1 );
    const char * type_string  = config_iget( config , "GROUP_RATE" , i , 2 );
    const char * min_max_file = config_iget( config , "GROUP_RATE" , i , 3 );
    
    group_rate_type * group_rate = group_rate_alloc( sched_history , time_vector , group_name , phase_string , type_string , min_max_file );
    hash_insert_hash_owned_ref( group_rates , group_name , group_rate , group_rate_free__);
  }


  
  for (i=0; i < config_get_occurences( config , "WELL_RATE" ); i++) {
    const char * group_name   = config_iget( config , "WELL_RATE" , i , 0 );
    const char * well_name    = config_iget( config , "WELL_RATE" , i , 1 );
    double corr_length        = config_iget_as_double( config , "WELL_RATE" , i , 2 );
    const char * stat_file    = config_iget( config , "WELL_RATE" , i , 3 );
    
    well_rate_type * well_rate;
    group_rate_type * group_rate = hash_get( group_rates , group_name );
    well_rate = well_rate_alloc( sched_history , time_vector , well_name , corr_length ,  stat_file , group_rate_get_phase( group_rate) , group_rate_is_producer( group_rate ));
    group_rate_add_well_rate( group_rate , well_rate );
  }
  
  {
    hash_iter_type * group_iter = hash_iter_alloc( group_rates );
    while (!hash_iter_is_complete( group_iter )) {
      group_rate_type * group_rate = hash_iter_get_next_value( group_iter );
      group_rate_init( group_rate );
    }
    hash_iter_free( group_iter );
  }
}




void sample( hash_type * group_rates ) {
  hash_iter_type * group_iter = hash_iter_alloc( group_rates );
  
  while (!hash_iter_is_complete( group_iter )) {
    group_rate_type * group_rate = hash_iter_get_next_value( group_iter );
    group_rate_sample( group_rate );
  }

  hash_iter_free( group_iter );
}

void debug (const time_t_vector_type * time_vector ) {
  stringlist_type * s1 = stringlist_alloc_new();
  stringlist_type * s2 = stringlist_alloc_new();
  fscanf_2ts( time_vector , "/d/proj/bg/oseberg2/ressim/aoreln2/2001b/pert_hist/stat/test5/OSB/B-29_OIL.stat" , s1 , s2 );

  for (int i = 0; i < stringlist_get_size( s1 ); i++) {
    util_fprintf_date( time_t_vector_iget( time_vector , i ) , stdout);
    printf("  %7s -> %7s  \n",stringlist_iget(s1 , i) , stringlist_iget(s2,i));
  }
  exit(1);
}


int main( int argc , char ** argv ) {
  hash_type   * group_rates = hash_alloc();
  config_parser_type * config      = config_alloc();
  char        * config_file;
  {
    char * config_base;
    char * config_ext;
    char * run_path;
    
    if (util_is_link( argv[1] )) {   /* The command line argument given is a symlink - we start by changing to */
                                     /* the real location of the configuration file. */
      char  * realpath = util_alloc_link_target( argv[1] ); 
      util_alloc_file_components(realpath , &run_path , &config_base , &config_ext);
      free( realpath );
    } else 
      util_alloc_file_components( argv[1] , &run_path , &config_base , &config_ext);
    
    if (run_path != NULL) {
      printf("Changing to directory: %s \n",run_path);
      if (chdir( run_path) != 0)
        util_exit("Hmmmm - failed to change to directory:%s \n",run_path);
    }
    config_file = util_alloc_filename(NULL , config_base , config_ext);
    util_safe_free( config_base );
    util_safe_free( config_ext );
    util_safe_free( run_path );
  }
  
  config_init( config );
  config_parse(config , config_file , "--" , NULL , "DEFINE" , false , true );
  {
    sched_history_type * sched_history = sched_history_alloc(":");
    const char * data_file       = config_iget( config , "DATA_FILE" , 0 , 0 );
    const char * sched_file_name = config_iget( config , "SCHEDULE_FILE" , 0 , 0 );
    path_fmt_type * sched_fmt    = path_fmt_alloc_path_fmt( config_iget( config , "TARGET" , 0 , 0) );
    const int num_realizations   = config_iget_as_int(config , "NUM_REALIZATIONS" , 0 , 0 );
    msg_type * msg               = msg_alloc("Creating file: ", false);
    
    time_t start_date = ecl_util_get_start_date( data_file );
    time_t_vector_type * time_vector;
    /* Loading input and creating well/group objects. */
    {
      sched_file_type * sched_file       = sched_file_parse_alloc( sched_file_name , start_date );
      sched_history_update( sched_history , sched_file );
      
      time_vector = sched_file_alloc_time_t_vector( sched_file );
      load_groups( config , sched_file ,group_rates , sched_history , time_vector );
      sched_file_free( sched_file );
    }

    
    /* Sampling and creating output */
    {
      int i;
      msg_show( msg );
      for (i = 0; i < num_realizations; i++) {
        //sched_file_type * sched_file = sched_file_alloc_copy( );
        sched_file_type * sched_file = sched_file_parse_alloc( sched_file_name , start_date );
        sample( group_rates );
        sched_file_update( sched_file , WCONHIST , perturb_wconhist , group_rates );
        sched_file_update( sched_file , WCONINJE , perturb_wconinje , group_rates );

        {
          char * new_file = path_fmt_alloc_file(sched_fmt , true , i );
          sched_file_fprintf( sched_file , new_file , false);
          msg_update( msg , new_file );
          free( new_file );
        }
        sched_file_free( sched_file );
      }
    }
    msg_free( msg , true );
    sched_history_free( sched_history );
  }
  config_free( config );
  hash_free( group_rates );
}
