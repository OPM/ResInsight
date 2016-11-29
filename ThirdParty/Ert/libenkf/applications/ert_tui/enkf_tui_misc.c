/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_misc.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/menu.h>
#include <ert/util/util.h>

#include <ert/job_queue/ext_joblist.h>
#include <ert/job_queue/ext_job.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_state.h>

#include <enkf_tui_misc.h>
#include <enkf_tui_help.h>

static void enkf_tui_misc_printf_subst_list(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );

  /* These could/should be user input ... */
  int step1 = 0;  /* < 0 => no reinitializtion of the dynamic substitutions. */
  int step2 = 10;
  int iens  = 0;
  
  enkf_state_type * enkf_state = enkf_main_iget_state( enkf_main , iens );
  enkf_state_printf_subst_list( enkf_state , step1 , step2 );
}


static void enkf_tui_misc_list_jobs(void * arg) {
  enkf_main_type * enkf_main              = enkf_main_safe_cast( arg );
  const ext_joblist_type * installed_jobs = enkf_main_get_installed_jobs( enkf_main );
  stringlist_type * job_names             = ext_joblist_alloc_list( installed_jobs );
  int job_nr;
  stringlist_sort( job_names , NULL );
  printf("================================================================================\n");
  printf("%-30s : Arguments\n" , "Job name");
  printf("--------------------------------------------------------------------------------\n");
  for (job_nr = 0; job_nr < stringlist_get_size( job_names ); job_nr++) {
    const ext_job_type * job = ext_joblist_get_job( installed_jobs , stringlist_iget( job_names , job_nr ));
    const stringlist_type * arglist = ext_job_get_arglist( job );
    printf("%-30s : " , stringlist_iget( job_names , job_nr ));
    if (arglist != NULL)
      stringlist_fprintf( arglist , " " , stdout );
    printf("\n");
  }
  printf("================================================================================\n");
  stringlist_free( job_names );
}




void enkf_tui_misc_menu( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );
  menu_type       * menu       = menu_alloc( "Misceallanous stuff" , "Back" , "bB");
  menu_add_item(menu , "List all \'magic\' <...> strings" , "lL"    , enkf_tui_misc_printf_subst_list , enkf_main , NULL); 
  menu_add_item(menu , "List all available forward model jobs","jJ" , enkf_tui_misc_list_jobs , enkf_main , NULL );
  menu_add_item(menu , "Help","hH" , enkf_tui_help_menu_misc , enkf_main , NULL );
  menu_run(menu);
  menu_free(menu);
}
