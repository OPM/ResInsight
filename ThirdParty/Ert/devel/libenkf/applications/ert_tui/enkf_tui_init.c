/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_init.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <util.h>
#include <ctype.h>
#include <menu.h>
#include <enkf_main.h>
#include <enkf_sched.h>
#include <enkf_types.h>
#include <enkf_tui_util.h>
#include <enkf_tui_init.h>
#include <enkf_state.h>
#include <enkf_node.h>
#include <enkf_fs.h>
#include <msg.h>
#include <ensemble_config.h>




void enkf_tui_init(enkf_main_type * enkf_main, bool all_members , bool all_parameters , bool interval ) {
  const int prompt_len                         = 35;
  const ensemble_config_type * ensemble_config = enkf_main_get_ensemble_config(enkf_main);
  int   ens_size                               = enkf_main_get_ensemble_size( enkf_main );
  int iens1, iens2;
  bool iens_valid = false;
  
  /* iens2 should be interpreted as __inclusive__ */
  if ( all_members ) {
    iens1 = 0;
    iens2 = ens_size - 1;
    iens_valid = true;
  } else {
    if( interval ) {
      char * iens1char = util_scanf_int_with_limits_return_char("First ensemble member in interval"  , prompt_len , 0 , ens_size - 1);
      if (strlen(iens1char)) {
	util_sscanf_int(iens1char , &iens1);
	char * iens2char = util_scanf_int_with_limits_return_char("Second ensemble member in interval" , prompt_len , iens1 , ens_size - 1);
	if (strlen(iens2char)) {
	  util_sscanf_int(iens2char , &iens2);
	  iens_valid = true;
	}
	free(iens2char);
      }
      free(iens1char);
    } else {
      char * iens1char = util_scanf_int_with_limits_return_char("Initialize ensemble member" , prompt_len , 0 , ens_size - 1);
      if (strlen(iens1char)) {
	util_sscanf_int(iens1char , &iens1);
	iens2 = iens1;
	iens_valid = true;
      }
      free(iens1char);
    }
  }
  
  if (iens_valid) {
    stringlist_type * param_list = NULL;
    if (all_parameters) 
      param_list = ensemble_config_alloc_keylist_from_var_type( ensemble_config , PARAMETER );
    else {
      const enkf_config_node_type * config_node = NULL;
      param_list = stringlist_alloc_new();
      config_node = enkf_tui_util_scanf_key(ensemble_config , prompt_len , INVALID , INVALID_VAR);
      if( config_node != NULL )
        stringlist_append_copy( param_list , enkf_config_node_get_key(config_node));
    }

    if (param_list != NULL) {
      enkf_main_initialize_from_scratch(enkf_main , param_list , iens1 , iens2);
      stringlist_free( param_list );
    }
  }
}


static void enkf_tui_init1(void * enkf_main) {
  enkf_tui_init(enkf_main, true , true , false);
}

static void enkf_tui_init2(void * enkf_main) {
  enkf_tui_init(enkf_main , true , false , false);
}

static void enkf_tui_init3(void * enkf_main) {
  enkf_tui_init(enkf_main , false , true , false);
}

static void enkf_tui_init4(void * enkf_main) {
  enkf_tui_init(enkf_main , false , false, false);
}

static void enkf_tui_init5(void * enkf_main) {
  enkf_tui_init(enkf_main , false , true, true);
}

static void enkf_tui_init6(void * enkf_main) {
  enkf_tui_init(enkf_main , false , false, true);
}

void enkf_tui_init_menu(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast(arg);

  menu_type * menu = menu_alloc("Initialize from scratch" , "Back" , "bB");
  menu_add_item(menu , "Initialize all members/all parameters"                  , "1" , enkf_tui_init1 , enkf_main , NULL);
  menu_add_item(menu , "Initialize all members/one  parameter"                  , "2" , enkf_tui_init2 , enkf_main , NULL);
  menu_add_item(menu , "Initialize one member/all parameters"                   , "3" , enkf_tui_init3 , enkf_main , NULL);
  menu_add_item(menu , "Initialize one member/one parameter"                    , "4" , enkf_tui_init4 , enkf_main , NULL);
  menu_add_item(menu , "Initialize interval of ensemble members/all parameters" , "5" , enkf_tui_init5 , enkf_main , NULL);
  menu_add_item(menu , "Initialize interval of ensemble members/one parameter"  , "6" , enkf_tui_init6 , enkf_main , NULL);  
  menu_run(menu);
  menu_free(menu);

}
