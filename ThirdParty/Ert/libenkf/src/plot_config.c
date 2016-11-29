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

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>

#include <ert/enkf/plot_config.h>
#include <ert/enkf/enkf_defaults.h>
#include <ert/enkf/config_keys.h>

/**
    Struct holding basic information used when plotting.
*/

struct plot_config_struct {
  char * plot_path;     /* All the plots will be saved as xxxx files in this directory. */
};

void plot_config_set_path(plot_config_type * plot_config , const char * plot_path) {
  plot_config->plot_path = util_realloc_string_copy(plot_config->plot_path , plot_path);
}

const char *  plot_config_get_path(const plot_config_type * plot_config ) {
  return plot_config->plot_path;
}

void plot_config_free( plot_config_type * plot_config) {
  free(plot_config->plot_path);
  free(plot_config);
}


/**
   The plot_config object is instantiated with the default values from enkf_defaults.h
*/
plot_config_type * plot_config_alloc_default() {
  plot_config_type * info        = util_malloc( sizeof * info );
  info->plot_path                = NULL;
  plot_config_set_path(info , DEFAULT_PLOT_PATH );
  return info;
}


void plot_config_init(plot_config_type * plot_config , const config_content_type * config ) {
  if (config_content_has_item( config , PLOT_PATH_KEY))
    plot_config_set_path( plot_config , config_content_get_value( config , PLOT_PATH_KEY ));
}


void plot_config_add_config_items( config_parser_type * config ) {
  config_add_key_value(config , PLOT_PATH_KEY         , false , CONFIG_STRING);
}


