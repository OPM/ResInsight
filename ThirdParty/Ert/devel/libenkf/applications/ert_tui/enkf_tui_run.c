/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_run.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/string_util.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_sched.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_analysis.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/analysis_config.h>
#include <ert/enkf/analysis_iter_config.h>

#include <enkf_tui_util.h>
#include <enkf_tui_fs.h>
#include <enkf_tui_analysis.h>
#include <ert_tui_const.h>
#include <enkf_tui_help.h>

/*
  Set runpath runtime - disabled.

static void enkf_tui_run_set_runpath(void * arg) {
  arg_pack_type * arg_pack = arg_pack_safe_cast( arg );
  model_config_type * model_config = arg_pack_iget_ptr(arg_pack , 0);
  menu_item_type    * item         = arg_pack_iget_ptr(arg_pack , 1);
  char runpath_fmt[256];
  printf("Give runpath format ==> ");
  scanf("%s" , runpath_fmt);
  model_config_set_runpath_fmt(model_config , runpath_fmt);
  {
    char * menu_label = util_alloc_sprintf("Set new value for RUNPATH:%s" , runpath_fmt);
    menu_item_set_label( item , menu_label );
    free(menu_label);
  }
}
*/



void enkf_tui_run_start(void * enkf_main) {
  const int ens_size = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc(0,true);
  bool_vector_iset( iactive , ens_size - 1 , true );

  enkf_main_run_assimilation(enkf_main , iactive , 0 , 0 , ANALYZED);
  
  bool_vector_free(iactive);
}



void enkf_tui_run_restart__(void * enkf_main) {
  const int ens_size    = enkf_main_get_ensemble_size( enkf_main );
  const int last_report = enkf_main_get_history_length( enkf_main );
  int start_report;
  char * start_report_as_char;
  bool wronginput = false;
  state_enum state;
  bool_vector_type * iactive = bool_vector_alloc(0,true);
  bool_vector_iset( iactive , ens_size - 1 , true );
  
  start_report_as_char = util_scanf_int_with_limits_return_char("Report step",PROMPT_LEN , 0 , last_report);
  if(strlen(start_report_as_char) != 0){
    util_sscanf_int(start_report_as_char , &start_report);
  }
  else
    wronginput = true;
  
  if(!wronginput){
    state        = enkf_tui_util_scanf_state("Analyzed/forecast" , PROMPT_LEN , false);
    if(state == UNDEFINED)
      wronginput = true;
  }
  
  if(!wronginput)
    enkf_main_run_assimilation(enkf_main ,  iactive , start_report , start_report  , state);
  
  bool_vector_free(iactive);
  free(start_report_as_char);
}



void enkf_tui_run_smoother(void * arg) {
  enkf_main_type * enkf_main  = enkf_main_safe_cast( arg );
  enkf_main_run_smoother(enkf_main , "AUTO-SMOOTHER" , true );
}



void enkf_tui_run_iterated_ES(void * enkf_main) {
  const int ens_size    = enkf_main_get_ensemble_size( enkf_main );
  const int last_report = enkf_main_get_history_length( enkf_main );

  {
    model_config_type * model_config = enkf_main_get_model_config( enkf_main ); 
    const ecl_config_type * ecl_config = enkf_main_get_ecl_config( enkf_main );
    const analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
    analysis_iter_config_type * iter_config = analysis_config_get_iter_config( analysis_config );
    analysis_module_type * module = analysis_config_get_active_module( analysis_config );
    int step1 = 0;
    int step2 ;
    int_vector_type * step_list = int_vector_alloc(0,0);
    bool_vector_type * iactive = bool_vector_alloc(0 , true);
    int iter  = 0;
    int num_iter = analysis_iter_config_get_num_iterations( iter_config );
    stringlist_type * node_list = ensemble_config_alloc_keylist_from_var_type( enkf_main_get_ensemble_config(enkf_main) , PARAMETER );

    if (ecl_config_has_schedule( ecl_config ))
      step2 = util_scanf_int_with_limits("Last report",PROMPT_LEN , 0 , last_report);  
    else
      step2 = last_report;
    
    {
      for (int step=step1; step <= step2; step++)
        int_vector_append( step_list , step );
    }
    bool_vector_iset( iactive , ens_size - 1 , true );
    
    while (true) {
      {
        const char * runpath_fmt = analysis_iter_config_iget_runpath_fmt( iter_config , iter);
        if (runpath_fmt != NULL) {
          char * runpath_key = util_alloc_sprintf( "runpath-%d" , iter);
          model_config_add_runpath( model_config , runpath_key , runpath_fmt);
          model_config_select_runpath( model_config , runpath_key );
          free( runpath_key );
        }
      }
      
      enkf_main_run_exp(enkf_main , iactive , true , step1 , step1 , FORECAST);
      {
        char * target_fs_name    = analysis_iter_config_iget_case( iter_config , iter );
        enkf_fs_type * target_fs = enkf_main_get_alt_fs(enkf_main , target_fs_name , false , true );
        enkf_main_smoother_update(enkf_main , step_list , target_fs);
          
        enkf_main_copy_ensemble( enkf_main , 
                                 enkf_main_get_current_fs( enkf_main ),
                                 0 ,   // Smoother update will write on step 0
                                 ANALYZED , 
                                 target_fs, 
                                 step1 , 
                                 FORECAST , 
                                 iactive , 
                                 NULL , 
                                 node_list );
        
        
        enkf_main_set_fs(enkf_main , target_fs , enkf_fs_get_case_name( target_fs ));
        free( target_fs_name );
      }
      //iter = analysis_module_get_int(module, "ITER");
      iter++;
      if (iter == num_iter)
        break;
    }
    int_vector_free( step_list );
    bool_vector_free( iactive );
  }
  
}




/** 
    Experiments will always start with the parameters at time == 0; if
    you want to simulate with updated (posterior) parameters, you
    ensure that by initializing from a report_step > 0 from an
    existing case.

    Prediction part is included if it exists.
*/



void enkf_tui_run_exp(void * enkf_main) {
  const int ens_size          = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive  = bool_vector_alloc(0,false);

  state_enum init_state    = ANALYZED; 
  int start_report         = 0;
  int init_step_parameters = 0;
  
  {

    char * prompt = util_alloc_sprintf("Which realizations to simulate (Ex: 1,3-5) <Enter for all> [M to return to menu] : " , ens_size);
    char * select_string;
      
    util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");
    select_string = util_alloc_stdin_line();
    enkf_tui_util_sscanf_active_list( iactive , select_string , ens_size);

    util_safe_free( select_string );
    free( prompt );
  }
  if (bool_vector_count_equal(iactive , true))
    enkf_main_run_exp(enkf_main , iactive , true , init_step_parameters , start_report , init_state);
  
  bool_vector_free(iactive);
}



void enkf_tui_run_create_runpath__(void * __enkf_main) {
  enkf_main_type * enkf_main = enkf_main_safe_cast(__enkf_main);
  const int ens_size         = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc(0,false);

  state_enum init_state    = ANALYZED; 
  int start_report         = 0;
  int init_step_parameters = 0;
  {
    char * prompt = util_alloc_sprintf("Which realizations to create[ensemble size:%d] : " , ens_size);
    char * select_string;

    util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");
    select_string = util_alloc_stdin_line();
    enkf_tui_util_sscanf_active_list( iactive , select_string , ens_size );
    
    util_safe_free( select_string );
    free( prompt );
  }
  enkf_main_run_exp(enkf_main , iactive , false , init_step_parameters , start_report , init_state);
  bool_vector_free(iactive);
}



static void enkf_tui_display_load_msg( int iens , const stringlist_type * msg_list ) {
  for (int i=0; i < stringlist_get_size( msg_list ); i++)
    printf("[%03d] : %s \n", iens , stringlist_iget( msg_list , i ));
}


void enkf_tui_run_manual_load__( void * arg ) {
  enkf_main_type * enkf_main                   = enkf_main_safe_cast( arg );
  enkf_fs_type * fs                            = enkf_main_get_fs( enkf_main ); 
  const int last_report                        = -1;
  const int ens_size                           = enkf_main_get_ensemble_size( enkf_main );
  int step1,step2;
  bool_vector_type * iactive = bool_vector_alloc( 0 , false );
  run_mode_type run_mode = ENSEMBLE_EXPERIMENT; 
  
  enkf_main_init_run(enkf_main , run_mode);     /* This is ugly */
  
  step1 = 0;
  step2 = last_report;  /** Observe that for the summary data it will load all the available data anyway. */
  {
    char * prompt = util_alloc_sprintf("Which realizations to load  (Ex: 1,3-5) <Enter for all> [M to return to menu] : [ensemble size:%d] : " , ens_size);
    char * select_string;
    util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");
    select_string = util_alloc_stdin_line();

    enkf_tui_util_sscanf_active_list( iactive , select_string , ens_size );
    util_safe_free( select_string );
    
    free( prompt );
  }



  if (bool_vector_count_equal( iactive , true )) {
    int iens;
    arg_pack_type ** arg_list = util_calloc( ens_size , sizeof * arg_list );
    thread_pool_type * tp = thread_pool_alloc( 4 , true );  /* num_cpu - HARD coded. */

    for (iens = 0; iens < ens_size; iens++) {
      arg_pack_type * arg_pack = arg_pack_alloc();
      arg_list[iens] = arg_pack;
      
      if (bool_vector_iget(iactive , iens)) {
        enkf_state_type * enkf_state = enkf_main_iget_state( enkf_main , iens );

        arg_pack_append_ptr( arg_pack , enkf_state);                                        /* 0: */
        arg_pack_append_ptr( arg_pack , fs );                                               /* 1: */
        arg_pack_append_int( arg_pack , step1 );                                            /* 2: This will be the load start parameter for the run_info struct. */
        arg_pack_append_int( arg_pack , step1 );                                            /* 3: Step1 */ 
        arg_pack_append_int( arg_pack , step2 );                                            /* 4: Step2 For summary data it will load the whole goddamn thing anyway.*/
        arg_pack_append_bool( arg_pack , true );                                            /* 5: Interactive */                  
        arg_pack_append_owned_ptr( arg_pack , stringlist_alloc_new() , stringlist_free__);  /* 6: List of interactive mode messages. */
        thread_pool_add_job( tp , enkf_state_internalize_results_mt , arg_pack);
        
      }
    }
    
    thread_pool_join( tp );
    thread_pool_free( tp );
    printf("\n");

    {
      qc_module_type * qc_module = enkf_main_get_qc_module( enkf_main );
      runpath_list_type * runpath_list = qc_module_get_runpath_list( qc_module );

      for (iens = 0; iens < ens_size; iens++) {
        if (bool_vector_iget(iactive , iens)) {
          const enkf_state_type * state = enkf_main_iget_state( enkf_main , iens );
          runpath_list_add( runpath_list , iens , enkf_state_get_run_path( state ) , enkf_state_get_eclbase( state ));
        }
      }

      qc_module_export_runpath_list( qc_module );
    }

    for (iens = 0; iens < ens_size; iens++) {
      if (bool_vector_iget(iactive , iens)) {
        stringlist_type * msg_list = arg_pack_iget_ptr( arg_list[iens] , 6 );
        if (stringlist_get_size( msg_list ))
          enkf_tui_display_load_msg( iens , msg_list );
      }
    }
    
    
    for (iens = 0; iens < ens_size; iens++) 
      arg_pack_free( arg_list[iens]);
    free( arg_list );      
  }
  bool_vector_free( iactive );
}






/*****************************************************************/

void enkf_tui_run_menu(void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );
  model_config_type * model_config = enkf_main_get_model_config( enkf_main );
  path_fmt_type     * runpath_fmt  = model_config_get_runpath_fmt( model_config );
  menu_type       * menu;

  {
    char   * title = util_alloc_sprintf("Run menu [case:%s  Runpath:%s]" , enkf_main_get_current_fs( enkf_main ) , path_fmt_get_fmt ( runpath_fmt ));
    menu = menu_alloc(title , "Back" , "bB");
    free(title);
  }
  menu_add_item(menu , "Ensemble run: history"                , "xX" , enkf_tui_run_exp         , enkf_main , NULL);
  menu_add_separator( menu );
  {
    const ecl_config_type * ecl_config = enkf_main_get_ecl_config( enkf_main );
    const model_config_type * model_config = enkf_main_get_model_config( enkf_main );
    
    menu_item_type * enkf_item         = menu_add_item(menu , "Start EnKF run from beginning"          , "sS" , enkf_tui_run_start         , enkf_main , NULL);
    menu_item_type * restart_enkf_item = menu_add_item(menu , "Restart EnKF run from arbitrary state"  , "rR" , enkf_tui_run_restart__       , enkf_main , NULL);
    menu_item_type * ES_item           = menu_add_item(menu , "Integrated smoother update"             , "iI" , enkf_tui_run_smoother      , enkf_main , NULL);
    menu_item_type * it_ES_item        = menu_add_item(menu , "Iterated smoother [RML-EnKF]"           , "tT" , enkf_tui_run_iterated_ES   , enkf_main , NULL);
              
    if (!ecl_config_has_schedule( ecl_config )) {
      menu_item_disable( enkf_item );
      menu_item_disable( restart_enkf_item );
    }

    if (!model_config_has_history( model_config )) {
      menu_item_disable( it_ES_item );
      menu_item_disable( ES_item );
    }
  }
  menu_add_separator(menu);
  menu_add_item(menu , "Create runpath directories - NO simulation" , "cC" , enkf_tui_run_create_runpath__ , enkf_main , NULL );
  menu_add_item(menu , "Load results manually"                               , "lL"  , enkf_tui_run_manual_load__ , enkf_main , NULL);
  menu_add_separator(menu);
  {
    menu_item_type * analysis_item = menu_add_item(menu , "Analysis menu"             , "aA" , enkf_tui_analysis_menu , enkf_main , NULL);
    
    if (!enkf_main_have_obs( enkf_main )) 
      menu_item_disable( analysis_item );
  }
  /*
    Option to set runpath runtime - currently dismantled.
    
    menu_add_separator(menu);
    {
    model_config_type * model_config = enkf_main_get_model_config( enkf_main );
    path_fmt_type     * runpath_fmt  = model_config_get_runpath_fmt( model_config );
    arg_pack_type * arg_pack = arg_pack_alloc();  
    char * runpath_label = util_alloc_sprintf("Set new value for RUNPATH:%s" , path_fmt_get_fmt ( runpath_fmt ));
    
    arg_pack_append_ptr(arg_pack , model_config);
    arg_pack_append_ptr(arg_pack , menu_add_item(menu , runpath_label , "dD" , enkf_tui_run_set_runpath , arg_pack , arg_pack_free__));
    
    free(runpath_label);
    }
  */
  menu_add_item(menu , "Help"                                  , "hH" , enkf_tui_help_menu_run   , enkf_main , NULL); 
  menu_run(menu);
  menu_free(menu);

}
