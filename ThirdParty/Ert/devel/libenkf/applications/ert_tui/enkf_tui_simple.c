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
#include <ctype.h>

#include <ert/util/util.h>
#include <ert/util/menu.h>
#include <ert/util/msg.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_sched.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/enkf_node.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/ecl_config.h>

#include <enkf_tui_util.h>
#include <enkf_tui_init.h>
#include <enkf_tui_run.h>
#include <enkf_tui_plot.h>
#include <enkf_tui_help.h>
#include <enkf_tui_main.h>


void enkf_tui_simple_menu(void * arg) {
  enkf_main_type * enkf_main = enkf_main_safe_cast(arg);
  menu_type * menu = menu_alloc("Simple menu" , "Quit" , "qQ");
  menu_add_item(menu , "Sensitivity run: No data conditioning"                , "sS" , enkf_tui_run_exp         , enkf_main , NULL);
  const ecl_config_type * ecl_config = enkf_main_get_ecl_config( enkf_main );
  const model_config_type * model_config = enkf_main_get_model_config( enkf_main );
  menu_item_type * enkf_item         = menu_add_item(menu , "Assimilation run: EnKF updates"                  , "eE" , enkf_tui_run_start         , enkf_main , NULL);
  menu_item_type * ES_item           = menu_add_item(menu , "Assimilation run: Smoother update"               , "aA" , enkf_tui_run_smoother      , enkf_main , NULL);
  menu_item_type * it_ES_item        = menu_add_item(menu , "Assimilation run: Iterated smoother [RML-EnKF]"  , "iI" , enkf_tui_run_iterated_ES   , enkf_main , NULL);
  if (!ecl_config_has_schedule( ecl_config )) {
    menu_item_disable( enkf_item );
  }
  if (!model_config_has_history( model_config )) {
    menu_item_disable( it_ES_item );
    menu_item_disable( ES_item );
  }
  menu_add_separator( menu );
  menu_add_item(menu , "Plot results"                          , "pP" , enkf_tui_plot_simple_menu   , enkf_main , NULL);
  {
      menu_item_type * menu_item = menu_add_item( menu , "Create pdf reports" , "rR" , enkf_tui_plot_reports , enkf_main , NULL );
      ert_report_list_type * report_list = enkf_main_get_report_list( enkf_main );
      
      if (ert_report_list_get_num( report_list ) == 0)
        menu_item_disable( menu_item );
      
  }
  menu_add_separator(menu); 
  menu_add_item(menu , "Help"                                  , "hH" , enkf_tui_help_menu_simple   , enkf_main , NULL);
  menu_add_item(menu , "Advanced menu"                         , "dD" , enkf_tui_main_menu        , enkf_main , NULL);  
  menu_run(menu);
  menu_free(menu);

}
