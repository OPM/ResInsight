/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'plot_config.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __PLOT_CONFIG_H__
#define __PLOT_CONFIG_H__

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>


typedef struct plot_config_struct plot_config_type;

void               plot_config_set_path(plot_config_type * plot_config , const char * plot_path);
const char  *      plot_config_get_path(const plot_config_type * plot_config );
void               plot_config_free( plot_config_type * plot_config);
plot_config_type * plot_config_alloc_default();
void               plot_config_init(plot_config_type * plot_config , const config_content_type * config );
void               plot_config_add_config_items( config_parser_type * config );

#endif
