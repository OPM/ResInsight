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

#include <ert/util/util.h>
#include <ert/util/menu.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_sched.h>

#include <enkf_tui_run.h>
#include <enkf_tui_export.h>
#include <enkf_tui_plot.h>
#include <enkf_tui_table.h>
#include <enkf_tui_fs.h>
#include <enkf_tui_ranking.h>
#include <enkf_tui_QC.h>
#include <enkf_tui_help.h>
#include <enkf_tui_misc.h>


void enkf_tui_help_manual_main( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");

  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s &" , browser , manual_url);
    system(cmd);
    free( cmd );
  }
}

void enkf_tui_help_menu_main(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Main menu" , "Back" , "bB");
    menu_add_helptext(menu , "Choose the different options from the main menu to read more about the different options.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_main , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_cases( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");

  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Manage_cases");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_cases(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Manage cases" , "Back" , "bB");
    menu_add_helptext(menu , "Use this menu to navgate between cases and to initialize cases. A case has to be initialized before it can be used.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_cases , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_run( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");
  
  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Run_or_restart_experiment");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_run(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Run menu" , "Back" , "bB");
    menu_add_helptext(menu , "This menu is used to do the main work in ERT. The first option, x: Ensemble run: history, will just run the case without any data conditioning. Options r and s will initiate classical enkf runs. The two options i and t invokes the ensemble kalman smoother.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_run , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_plot( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");
  
  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Plot_Results");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_plot(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Plot results" , "Back" , "bB");
    menu_add_helptext(menu , "Use this option to plot results after the case has been run.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_plot , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_rank( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");
  
  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Rank_Results");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_rank(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Rank results" , "Back" , "bB");
    menu_add_helptext(menu , "Use this option to rank results after the case has been run.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_rank , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_export( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");
  
  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Export_data_to_other_formats");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_export(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Export results" , "Back" , "bB");
    menu_add_helptext(menu , "Use this option to export results after the case has been run.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_export , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_table( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");
  
  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Table_of_results");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_table(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Table of results" , "Back" , "bB");
    menu_add_helptext(menu , "This option can generate a table of results after the case has been run.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_table , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_manual_misc( void * arg) {
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  const site_config_type * site_config = enkf_main_get_site_config ( enkf_main );
  const char * manual_url = site_config_get_manual_url( site_config );
  const char * browser = getenv("BROWSER");
  
  if (browser == NULL)
    browser = site_config_get_default_browser( site_config );
  
  {
    char * cmd = util_alloc_sprintf("%s %s%s &" , browser , manual_url , "#Miscellaneous");
    system(cmd);
    free( cmd );
  } 
}

void enkf_tui_help_menu_misc(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Miscellanous" , "Back" , "bB");
    menu_add_helptext(menu , "This option contains miscellaneous options.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_misc , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

void enkf_tui_help_menu_simple(void * arg) {
  
  enkf_main_type  * enkf_main  = enkf_main_safe_cast( arg );  
  plot_config_type * plot_config = enkf_main_get_plot_config( enkf_main );
  {
    const char * plot_path  =  plot_config_get_path( plot_config );
    util_make_path( plot_path );
  }
  
  {
    menu_type * menu = menu_alloc("Help: Simple menu" , "Back" , "bB");
    menu_add_helptext(menu , "The top four options in the simple menu will run eclipse simulations. Option s: Sensitivity run: No data conditioning, will initialize all parameters and run one eclipse simulation for each set of different parameters. e: Assimilation run: EnKF updates, will initialize all parameters and run one eclipse simulation for each set of different parameters. The eclipse simulations will run until the first data time step is encountered and condition on data before continuing. a: Assimilation run: Smoother update, will do one pass of the sensitivity run, then condition the parameters to all the data and rerun all experiments. i: Assimilation run: Iterated smoother [RML-EnKF] will iterate the smoother run several times.");
    menu_add_item(menu , "Open manual (firefox)"    , "mM"  , enkf_tui_help_manual_main , enkf_main , NULL);
    menu_run(menu);
    menu_free(menu);
  }
}

