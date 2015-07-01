/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_tui_QC.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <math.h>
#include <time.h>

#include <ert/util/double_vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/util.h>
#include <ert/util/menu.h>
#include <ert/util/arg_pack.h>
#include <ert/util/path_fmt.h>
#include <ert/util/bool_vector.h>
#include <ert/util/msg.h>
#include <ert/util/vector.h>
#include <ert/util/matrix.h>
#include <ert/util/type_vector_functions.h>

#include <ert/plot/plot.h>
#include <ert/plot/plot_dataset.h> 

#include <ert/ecl/ecl_rft_file.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/block_obs.h>
#include <ert/enkf/gen_obs.h>
#include <ert/enkf/field_config.h>
#include <ert/enkf/obs_vector.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/gen_kw_config.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/plot_config.h>
#include <ert/enkf/member_config.h>
#include <ert/enkf/enkf_analysis.h>
#include <ert/enkf/pca_plot_data.h>

#include <enkf_tui_util.h>
#include <enkf_tui_plot.h>
#include <enkf_tui_fs.h>
#include <ert_tui_const.h>
#include <enkf_tui_plot_util.h>



void enkf_tui_QC_plot_PC_list( void * arg ) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  stringlist_type * all_obs_keys   = enkf_obs_alloc_keylist( enkf_main_get_obs( enkf_main ));
  stringlist_type * obs_keys = stringlist_alloc_new();

  {
    char * keys_input;
    util_printf_prompt("Observation keys (wildcards allowed) - [default: all]", PROMPT_LEN, '=', "=> ");
    keys_input = util_alloc_stdin_line();
    if (keys_input)
    {
      stringlist_type * pattern_list = stringlist_alloc_from_split(keys_input, " ,");
      for (int i = 0; i < stringlist_get_size(pattern_list); i++)
      {
        const char * pattern = stringlist_iget(pattern_list, i);
        stringlist_append_matching_elements(obs_keys, all_obs_keys, pattern);
      }
      free(keys_input);
    }
  }

  if (stringlist_get_size(obs_keys) > 0)
  {
    const int last_report = enkf_main_get_history_length(enkf_main);
    vector_type * PC_list = vector_alloc_new();
    const int ncomp = 1;

    for (int iobs = 0; iobs < stringlist_get_size(obs_keys); iobs++)
    {
      local_obsdata_node_type * obsnode = local_obsdata_node_alloc(stringlist_iget(obs_keys, iobs));
      local_obsdata_type * obsdata = local_obsdata_alloc_wrapper(obsnode);
      local_obsdata_node_add_range(obsnode, 0, last_report);
      {
        pca_plot_data_type * plot_data = enkf_main_alloc_pca_plot_data(enkf_main, obsdata, ncomp);
        vector_append_owned_ref(PC_list, plot_data, pca_plot_data_free__);
      }
      local_obsdata_free(obsdata);
    }
    enkf_tui_plot_PC_list(enkf_main , PC_list);
    vector_free(PC_list);
  } else
    printf("Sorry: no observation keys mathced the pattern(s).\n");

  stringlist_free( obs_keys );
  stringlist_free( all_obs_keys );
}



void enkf_tui_QC_plot_PC( void * arg ) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const int last_report                  = enkf_main_get_history_length( enkf_main );
  int step1,step2;
  double truncation_or_ncomp;
  local_obsdata_type * obsdata = local_obsdata_alloc("PCA Observations");
  char * keys_input;
  
  
  enkf_tui_util_scanf_report_steps(last_report , PROMPT_LEN , &step1 , &step2);
  util_printf_prompt("Observation keys (wildcards allowed) - [default: all]" , PROMPT_LEN , '=' , "=> ");
  keys_input = util_alloc_stdin_line();

  util_printf_prompt("Truncation: [0,1): Explained variance  [1,ens_size): fixed" , PROMPT_LEN , '=' , "=> ");

  {
    char * input = util_alloc_stdin_line();

    if (input == NULL)
      return;
    else {
      if (!util_sscanf_double( input , &truncation_or_ncomp)) {
        fprintf(stderr , "Failed to parse:%s as number \n",input);
        free( input );
        return;
      }
    }
        
    free( input );
  }
  
  {
    stringlist_type * all_keys = enkf_obs_alloc_keylist( enkf_main_get_obs( enkf_main ));
    stringlist_type * obs_keys = stringlist_alloc_new();

    if (keys_input) {
      stringlist_type * input_keys = stringlist_alloc_from_split( keys_input , " ");
      int i;
      for (i=0; i < stringlist_get_size( input_keys ); i++)
        stringlist_append_matching_elements( obs_keys , all_keys , stringlist_iget( input_keys , i ));
      stringlist_free( input_keys );
    } else 
      stringlist_deep_copy( obs_keys , all_keys );




    {
      int iobs;
      
      for (iobs = 0; iobs < stringlist_get_size( obs_keys); iobs++) {
        const char * obs_key = stringlist_iget( obs_keys , iobs );
        if (!local_obsdata_has_node( obsdata , obs_key )) {
          local_obsdata_node_type * obs_node = local_obsdata_node_alloc( obs_key );

          local_obsdata_node_add_range( obs_node , step1 , step2 );
          local_obsdata_add_node( obsdata , obs_node );
        }
      }
      
      stringlist_free( all_keys );
      stringlist_free( obs_keys );
    } 
  }  
  
  if (local_obsdata_get_size( obsdata )) {
    matrix_type * PC     = matrix_alloc(1,1);
    matrix_type * PC_obs = matrix_alloc(1,1);
    analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
    char * plot_name = util_alloc_sprintf(analysis_config_get_PC_filename( analysis_config ) , 
                                          step1 , step2 , "obs");

    pca_plot_data_type * plot_data = enkf_main_alloc_pca_plot_data( enkf_main , obsdata , truncation_or_ncomp);
    
    enkf_tui_plot_PC( enkf_main , plot_name , plot_data );
    
    free( plot_name );
    matrix_free( PC );
    matrix_free( PC_obs );
    pca_plot_data_free( plot_data );
  }
  local_obsdata_free( obsdata );
}


void enkf_tui_QC_run_workflow( void * arg ) {
  enkf_main_type  * enkf_main        = enkf_main_safe_cast( arg );  
  const qc_module_type  * qc_module  = enkf_main_get_qc_module( enkf_main );
  
  qc_module_run_workflow( qc_module , enkf_main );
}



void enkf_tui_QC_menu(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Quality check of prior" , "Back" , "bB");
    menu_item_type * plot_PC_item         = menu_add_item( menu , "Plot of prior principal components"    , "pP"  , 
                                                           enkf_tui_QC_plot_PC , enkf_main , NULL);

    menu_item_type * plot_PC_list_item    = menu_add_item( menu , "Plot first principal component for all observations" , "aA" , 
                                                            enkf_tui_QC_plot_PC_list, enkf_main , NULL);

    menu_item_type * run_QC_workflow_item = menu_add_item( menu , "Run QC workflow"    , "rR"  , enkf_tui_QC_run_workflow , enkf_main , NULL);
    
    if (!enkf_main_have_obs( enkf_main )) {
      menu_item_disable( plot_PC_item );
      menu_item_disable( plot_PC_list_item );
    }
    
    if (!enkf_main_has_QC_workflow( enkf_main ))
      menu_item_disable( run_QC_workflow_item );
    
    menu_run(menu);
    menu_free(menu);
  }
}

