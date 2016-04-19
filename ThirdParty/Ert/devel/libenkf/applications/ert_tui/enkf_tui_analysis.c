/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_analysis.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>
#include <ert/util/stringlist.h>

#include <ert/analysis/analysis_module.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/analysis_config.h>

#include <enkf_tui_util.h>
#include <enkf_tui_fs.h>
#include <ert_tui_const.h>



void enkf_tui_analysis_scale_observation_std__(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast(arg);

  double scale_factor = enkf_tui_util_scanf_double_with_lower_limit("Global scaling factor", PROMPT_LEN, 0);

  if (enkf_main_have_obs(enkf_main)) {
    enkf_obs_type * observations = enkf_main_get_obs(enkf_main);
    enkf_obs_scale_std(observations, scale_factor);
  }
}






static void enkf_tui_analysis_update_title( enkf_main_type * enkf_main , menu_type * menu ) {
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
  analysis_module_type * analysis_module = analysis_config_get_active_module( analysis_config );
  char * title = util_alloc_sprintf("Analysis menu [Current module:%s]" , analysis_module_get_name( analysis_module ));
  menu_set_title( menu , title );
  free( title );
}


void enkf_tui_analysis_select_module__(void * arg) {
  int prompt_len = 50;
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  enkf_main_type * enkf_main = arg_pack_iget_ptr( arg_pack , 0 );
  menu_type * menu = arg_pack_iget_ptr( arg_pack , 1 );
  
  {
    analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
    char module_name[256];
    util_printf_prompt("Name module to select" , prompt_len , '=' , "=> ");
    scanf("%s", module_name);
    if (analysis_config_select_module( analysis_config , module_name ))
      enkf_tui_analysis_update_title( enkf_main , menu );
  }
}


void enkf_tui_analysis_list_modules__(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );

  printf("Available modules: ");
  {
    stringlist_type * modules = analysis_config_alloc_module_names( analysis_config );
    stringlist_fprintf( modules , " " , stdout );
    printf("\n");
    stringlist_free( modules );
  }
}


void enkf_tui_analysis_reload_module__(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
  analysis_config_reload_module( analysis_config , NULL );
}


void enkf_tui_analysis_update_module__(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  int prompt_len = 50;
  {
    analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
    analysis_module_type * analysis_module = analysis_config_get_active_module( analysis_config );
    char var_name[256];
    char value[256];
    
    util_printf_prompt("Variable to modify" , prompt_len , '=' , "=> "); 
    scanf("%s", var_name);
    {
      char * value_prompt = util_alloc_sprintf("New value for %s" , var_name);
      util_printf_prompt(value_prompt , prompt_len , '=' , "=> "); 
      free( value_prompt );
    }
    scanf("%s", value);

    if (analysis_module_set_var( analysis_module , var_name , value))
      printf("\'%s\' successfully set to \'%s\' \n",var_name , value);
    else
      printf("** Variable/type combination: %s/%s not recognized \n", var_name , value);
    
  }
}




void enkf_tui_analysis_menu(void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );
  menu_type       * menu = menu_alloc( "Analysis menu" , "Back" , "bB");

  arg_pack_type * arg_pack = arg_pack_alloc();
  arg_pack_append_ptr( arg_pack , enkf_main );
  arg_pack_append_ptr( arg_pack , menu );

  {
    enkf_tui_analysis_update_title(enkf_main, menu);

    if (enkf_main_have_obs(enkf_main)) {
      menu_add_item(menu, "Global scaling of uncertainty", "gG", enkf_tui_analysis_scale_observation_std__, enkf_main, NULL);
      menu_add_separator(menu);
    }

    menu_add_item(menu, "Select analysis module", "sS", enkf_tui_analysis_select_module__, arg_pack, NULL);
    menu_add_item(menu, "List available modules", "lL", enkf_tui_analysis_list_modules__, enkf_main, NULL);
    menu_add_item(menu, "Modify analysis module parameters", "mM", enkf_tui_analysis_update_module__, enkf_main, NULL);
    menu_add_item(menu, "Reload current module (external only)", "rR", enkf_tui_analysis_reload_module__, enkf_main, NULL);
  }
  menu_run(menu);
  menu_free(menu);
  arg_pack_free(arg_pack);
}

