/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'plot_settings.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_PLOT_SETTINGS_H
#define ERT_PLOT_SETTINGS_H

#include <ert/config/config_parser.h>
#include <ert/config/config_settings.h>

void               plot_settings_init(config_settings_type * setting);
void               plot_settings_add_config_items( config_parser_type * config );

#endif
