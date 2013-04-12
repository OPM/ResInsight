/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_tui_workflow.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/bool_vector.h>

#include <ert/config/config_error.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_sched.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_analysis.h>
#include <ert/enkf/ecl_config.h>

#include <enkf_tui_analysis.h>
#include <ert_tui_const.h>
#include <enkf_tui_help.h>
#include <enkf_tui_util.h>
#include <enkf_tui_fs.h>


void enkf_tui_workflow_run( void * arg ) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  {
    ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );
    util_printf_prompt("Name of workflow" , PROMPT_LEN , '=' , "=> ");
    {
      char * workflow_name = util_alloc_stdin_line();
      if (workflow_name != NULL) {
        if (ert_workflow_list_has_workflow( workflow_list , workflow_name )) {
          if (!ert_workflow_list_run_workflow( workflow_list , workflow_name , enkf_main )) {
            printf("Errors in workflow:%s \n", workflow_name );
            printf("-----------------------------------------------------------------\n");
            config_error_fprintf( ert_workflow_list_get_last_error( workflow_list ) , true , stdout);
            printf("-----------------------------------------------------------------\n");
          }
        }
      }
      util_safe_free( workflow_name );
    }
  }
}


void enkf_tui_workflow_load( void * arg ) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  {
    
  }
}


void enkf_tui_workflow_list( void * arg ) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  {
    ert_workflow_list_type * workflow_list = enkf_main_get_workflow_list( enkf_main );
    stringlist_type * name_list = ert_workflow_list_alloc_namelist( workflow_list );
    
    printf("Available workflows: \n");
    {
      int i;
      for (i=0; i < stringlist_get_size( name_list ); i++) {
        if ((i % 5) == 0)
          printf("\n   ");
        else
          printf("  ");

        printf( stringlist_iget( name_list , i ));
      }
    }
    stringlist_free( name_list );
    printf("\n\n");
  }
}


void enkf_tui_workflow_menu(void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );
  menu_type       * menu  = menu_alloc("Workflows" , "Back" , "bB");
  
  menu_add_item(menu , "Run workflow"  , "rR" , enkf_tui_workflow_run , enkf_main , NULL );
  menu_add_item(menu , "Load workflow" , "lL" , enkf_tui_workflow_load , enkf_main , NULL );
  menu_add_item(menu , "List available workflows" , "iI" , enkf_tui_workflow_list , enkf_main , NULL );
  
  menu_add_item(menu , "Help" , "hH" , enkf_tui_help_menu_run   , enkf_main , NULL); 
  menu_run(menu);
  menu_free(menu);

}

