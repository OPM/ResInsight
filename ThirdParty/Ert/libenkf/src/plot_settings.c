/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'plot_config.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>

#include <ert/config/config_settings.h>
#include <ert/config/config_parser.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_content.h>

#include <ert/enkf/plot_settings.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>

#define TRUE_STRING              "True"
#define FALSE_STRING             "False"

#define PATH_KEY                 "PATH"
#define SHOW_REFCASE_KEY         "SHOW_REFCASE"
#define SHOW_HISTORY_KEY         "SHOW_HISTORY"

#define DEFAULT_PLOT_PATH        "plots"
#define DEFAULT_SHOW_REFCASE     FALSE_STRING
#define DEFAULT_SHOW_HISTORY     FALSE_STRING


void plot_settings_init(config_settings_type * settings) {

  config_settings_add_setting(settings , PATH_KEY , CONFIG_STRING , DEFAULT_PLOT_PATH );
  config_settings_add_setting(settings , SHOW_REFCASE_KEY , CONFIG_BOOL , DEFAULT_SHOW_REFCASE );
  config_settings_add_setting(settings , SHOW_HISTORY_KEY , CONFIG_BOOL , DEFAULT_SHOW_HISTORY );

}


void plot_settings_add_config_items( config_parser_type * config ) {
  config_settings_init_parser__( PLOT_SETTING_KEY , config , false );

  config_add_key_value(config , PLOT_PATH_KEY         , false , CONFIG_STRING);
  {
    char * msg = util_alloc_sprintf( "The keyword %s has been deprecated - use %s %s <PATH>", PLOT_PATH_KEY , PLOT_SETTING_KEY , PATH_KEY );
    config_parser_deprecate( config , PLOT_PATH_KEY , msg);
    free( msg );
  }
}


