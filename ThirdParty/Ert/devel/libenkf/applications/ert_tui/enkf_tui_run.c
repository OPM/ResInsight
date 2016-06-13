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
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_analysis.h>
#include <ert/enkf/ecl_config.h>
#include <ert/enkf/run_arg.h>

#include <enkf_tui_util.h>
#include <enkf_tui_fs.h>
#include <enkf_tui_analysis.h>
#include <ert_tui_const.h>
#include <enkf_tui_help.h>





void enkf_tui_run_smoother(void * arg) {
  enkf_main_type * enkf_main  = enkf_main_safe_cast( arg );
  int ens_size = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc( ens_size , true );
  enkf_fs_type * source_fs = enkf_main_tui_get_fs( enkf_main );
  enkf_main_run_smoother(enkf_main , source_fs , "AUTO-SMOOTHER" , iactive , 0 , true );
  bool_vector_free( iactive );
}



void enkf_tui_run_iterated_ES(void * arg) {
  enkf_main_type * enkf_main  = enkf_main_safe_cast( arg );
  const analysis_config_type * analysis_config = enkf_main_get_analysis_config(enkf_main);
  analysis_iter_config_type * iter_config = analysis_config_get_iter_config(analysis_config);
  int num_iter = analysis_iter_config_get_num_iterations(iter_config);
  enkf_main_run_iterated_ES(enkf_main , num_iter );
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
    enkf_main_run_exp(enkf_main , iactive );

  bool_vector_free(iactive);
}



void enkf_tui_run_create_runpath__(void * __enkf_main) {
  enkf_main_type * enkf_main = enkf_main_safe_cast(__enkf_main);
  const int ens_size         = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc(0,false);

  {
    char * prompt = util_alloc_sprintf("Which realizations to create[ensemble size:%d] : " , ens_size);
    char * select_string;

    util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");
    select_string = util_alloc_stdin_line();
    enkf_tui_util_sscanf_active_list( iactive , select_string , ens_size );

    util_safe_free( select_string );
    free( prompt );
  }
  enkf_main_create_run_path(enkf_main , iactive , 0 );
  bool_vector_free(iactive);
}



static void enkf_tui_display_load_msg( int iens , const stringlist_type * msg_list ) {
  for (int i=0; i < stringlist_get_size( msg_list ); i++)
    printf("[%03d] : %s \n", iens , stringlist_iget( msg_list , i ));
}


void enkf_tui_run_manual_load__( void * arg ) {
  enkf_main_type * enkf_main = enkf_main_safe_cast( arg );
  const int ens_size         = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive = bool_vector_alloc( 0 , false );
  int iter = 0;

  {
    char * prompt = util_alloc_sprintf("Which realizations to load  (Ex: 1,3-5) <Enter for all> [M to return to menu] : [ensemble size:%d] : " , ens_size);
    char * select_string;
    util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");
    select_string = util_alloc_stdin_line();

    enkf_tui_util_sscanf_active_list( iactive , select_string , ens_size );
    util_safe_free( select_string );

    free( prompt );
  }

  {
    const model_config_type * model_config = enkf_main_get_model_config( enkf_main );
    if (model_config_runpath_requires_iter( model_config )) {
      const char * prompt = "Which iteration to load from [0...?) : ";
      char * input;
      bool OK;
      util_printf_prompt(prompt , PROMPT_LEN , '=' , "=> ");

      input = util_alloc_stdin_line();
      if (input == NULL)
        return;

      OK = util_sscanf_int( input , &iter );

      free( input );
      if (!OK)
        return;
    }
  }


  if (bool_vector_count_equal( iactive , true )) {
    stringlist_type ** realizations_msg_list = util_calloc( ens_size , sizeof * realizations_msg_list );
    int iens = 0;
    for (; iens < ens_size; ++iens) {
      realizations_msg_list[iens] = stringlist_alloc_new();
    }

    enkf_main_load_from_forward_model(enkf_main, iter , iactive, realizations_msg_list);

    for (iens = 0; iens < ens_size; ++iens) {
      stringlist_type * msg_list = realizations_msg_list[iens];
      if (bool_vector_iget(iactive, iens)) {
        if (stringlist_get_size( msg_list )) {
          enkf_tui_display_load_msg( iens , msg_list );
        }
      }
      stringlist_free(msg_list);
    }
    free(realizations_msg_list);
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
  {
    const analysis_config_type * analysis_config = enkf_main_get_analysis_config(enkf_main);
    const enkf_obs_type * enkf_obs = enkf_main_get_obs( enkf_main );


    menu_item_type * ES_item           = menu_add_item(menu , "Integrated smoother update"             , "iI" , enkf_tui_run_smoother      , enkf_main , NULL);
    menu_item_type * it_ES_item        = menu_add_item(menu , "Iterated smoother [RML-EnKF]"           , "tT" , enkf_tui_run_iterated_ES   , enkf_main , NULL);

    if (!analysis_config_get_module_option(analysis_config , ANALYSIS_ITERABLE)) {
      menu_item_disable( it_ES_item );
    } else
      menu_item_disable( ES_item );

    if (!enkf_obs_have_obs( enkf_obs )) {
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
  menu_add_item(menu , "Help"                                  , "hH" , enkf_tui_help_menu_run   , enkf_main , NULL);
  menu_run(menu);
  menu_free(menu);
}
