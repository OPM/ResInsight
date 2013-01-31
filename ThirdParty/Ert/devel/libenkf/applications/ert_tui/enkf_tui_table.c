/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_table.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <ert/util/util.h>
#include <ert/util/menu.h>
#include <ert/util/arg_pack.h>
#include <ert/util/msg.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/block_obs.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/obs_vector.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/gen_kw_config.h>

#include <enkf_tui_util.h>
#include <enkf_tui_help.h>
#include <enkf_tui_plot.h>
#include <enkf_tui_fs.h>


        
           

static void enkf_tui_table__(enkf_main_type * enkf_main , bool gen_kw_table , bool ens_plot) {
  const ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config(enkf_main);
  enkf_fs_type               * fs              = enkf_main_get_fs(enkf_main);
  const int ens_size    = enkf_main_get_ensemble_size( enkf_main );
  const int last_report = enkf_main_get_history_length( enkf_main );
  int iens1, iens2, step1 , step2;
  int ikey , num_keys;
  int length;
  FILE * stream = NULL;
  int  * index;
  char ** user_keys;
  char ** index_keys;
  double ** data;
  bool     * active;  
  enkf_config_node_type ** config_nodes;
  enkf_node_type        ** nodes;

  const int prompt_len = 50;
  const char * keylist_prompt  = "Table headings: KEY1:INDEX1   KEY2:INDEX2 ....";
  const char * gen_kw_prompt   = "GEN_KW Parameter";
  const char * file_prompt     = "File to save in (blank for nothing) ";
  
  if (gen_kw_table) {
    char * key;
    enkf_config_node_type * config_node;
    util_printf_prompt(gen_kw_prompt , prompt_len , '=' , "=> ");
    key  = util_alloc_stdin_line();
    if (ensemble_config_has_key( ensemble_config , key )) {
      config_node = ensemble_config_get_node( ensemble_config , key );
      if (enkf_config_node_get_impl_type( config_node ) == GEN_KW) {
        gen_kw_config_type *  gen_kw_config = enkf_config_node_get_ref( config_node );
        num_keys                            = gen_kw_config_get_data_size( gen_kw_config );
        
        user_keys = util_calloc( num_keys , sizeof * user_keys );
        for (int i=0; i < num_keys; i++) 
          user_keys[i] = gen_kw_config_alloc_user_key( gen_kw_config , i);
        
      } else {
        fprintf(stderr,"wrong type of: %s \n",key);
        free( key );
        return; /* going home on invalid input */
      }
    } else {
      fprintf(stderr,"** warning: do not have key:%s \n", key);
      free( key );
      return ; /* going home on invalid input */
    }
    free( key );
  } else {
    char * input_keys;
    util_printf_prompt(keylist_prompt , prompt_len , '=' , "=> ");
    input_keys = util_alloc_stdin_line();
    util_split_string(input_keys , " " , &num_keys , &user_keys);
    free( input_keys );
  }
  

  util_printf_prompt(file_prompt , prompt_len , '=' , "=> ");
  {
    char * filename = util_alloc_stdin_line( );
    if (filename != NULL)
      stream = util_mkdir_fopen( filename , "w");
    free( filename );
  }

  active       = util_calloc( num_keys , sizeof * active       );
  nodes        = util_calloc( num_keys , sizeof * nodes        );
  config_nodes = util_calloc( num_keys , sizeof * config_nodes );
  index_keys   = util_calloc( num_keys , sizeof * index_keys   );
  for (ikey  = 0; ikey < num_keys; ikey++) {
    config_nodes[ikey] = (enkf_config_node_type *) ensemble_config_user_get_node( ensemble_config , user_keys[ikey] , &index_keys[ikey]);
    if (config_nodes[ikey] != NULL) {
      nodes[ikey]  = enkf_node_alloc( config_nodes[ikey] );
      active[ikey] = true;
    } else {
      fprintf(stderr,"** Warning: could not lookup node: %s \n",user_keys[ikey]);
      nodes[ikey]  = NULL;
      active[ikey] = false;
    }
  }
  
  if (ens_plot) {
    iens1  = 0;
    iens2  = enkf_main_get_ensemble_size( enkf_main );
    step1  = util_scanf_int_with_limits("report step",prompt_len , 0 , last_report);
    step2  = step1 + 1;
    length = (iens2 - iens1);
  } else {
    iens1  = util_scanf_int_with_limits("ensemble member",prompt_len , 0 , ens_size - 1);
    iens2  = iens1 + 1;
    step1  = 0;
    step2  = last_report + 1;
    length = (step2 - step1);
  }
  index = util_calloc( length   , sizeof * index );
  data  = util_calloc( num_keys , sizeof * data  );
  {
    int i;
    for (i = 0; i < num_keys; i++)
      data[i] = util_calloc( length , sizeof * data[i] );
  }
  
  {
    state_enum state  = FORECAST;
    int active_length = 0;
    int total_line_count = 0;
    double line[num_keys];
    int iens, step;
    
    for (iens = iens1; iens < iens2; iens ++) {
      for (step = step1; step < step2; step++) {
        int line_count = 0;
        
        for (ikey = 0; ikey < num_keys; ikey++) {
          if (active[ikey]) {
            node_id_type node_id = {.report_step = step, 
                                    .iens = iens , 
                                    .state = state };
            if (enkf_node_user_get( nodes[ikey] , fs , index_keys[ikey] , node_id , &line[ikey]))
              line_count++;
            else
              line[ikey] = -1;
          }
        }
        
        if (line_count > 0) {
          for (ikey=0; ikey < num_keys; ikey++) 
            data[ikey][active_length] = line[ikey];
          index[active_length] = total_line_count;
          active_length++;
        }
        
        total_line_count++;
      }
    }
    
    if (stream != NULL) {
      if (ens_plot) 
        enkf_util_fprintf_data( index , (const double **) data , "Realization"   , (const char **) user_keys , active_length , num_keys , active , true , stream);
      else
        enkf_util_fprintf_data( index , (const double **) data , "Report-step" , (const char **) user_keys , active_length , num_keys , active , false , stream);
      fclose(stream);
    }

    printf("\n\n"); 
    if (ens_plot) 
      enkf_util_fprintf_data( index , (const double **) data , "Realization"   , (const char **) user_keys , active_length , num_keys , active , true , stdout);
    else
      enkf_util_fprintf_data( index , (const double **) data , "Report-step" , (const char **) user_keys , active_length , num_keys , active , false , stdout);
  }

  for (ikey = 0; ikey < num_keys; ikey++) {
    if (active[ikey])
      enkf_node_free( nodes[ikey] );

    free(index_keys[ikey]);
    free(user_keys[ikey]);
    free(data[ikey]);
  }
  free( user_keys );
  free( active );
  free( index_keys);
  free( data );
  free( nodes );
  free( config_nodes );
}





static void enkf_tui_table_ensemble(void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  enkf_tui_table__(enkf_main , false , true);
}


static void enkf_tui_table_GEN_KW_ensemble(void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  enkf_tui_table__(enkf_main , true , true);
}


static void enkf_tui_table_time(void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  enkf_tui_table__(enkf_main , false , false);
}





void enkf_tui_table_menu(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  {
    menu_type * menu = menu_alloc("Table of results" , "Back" , "bB");
    menu_add_item(menu , "Ensemble of parameters"          , "eE"  , enkf_tui_table_ensemble        , enkf_main , NULL);
    menu_add_item(menu , "GEN_KW ensemble"                 , "gG"  , enkf_tui_table_GEN_KW_ensemble , enkf_main , NULL);
    menu_add_item(menu , "Time development of parameters"  , "tT"  , enkf_tui_table_time            , enkf_main , NULL);
    menu_add_item(menu , "Help"                            , "hH"  , enkf_tui_help_menu_table            , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}
