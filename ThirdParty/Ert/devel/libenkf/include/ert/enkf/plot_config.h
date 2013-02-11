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
#include <ert/config/config.h>

typedef struct plot_config_struct plot_config_type;

void               plot_config_set_width(plot_config_type * plot_config , int width);
void               plot_config_set_height(plot_config_type * plot_config , int height);
void               plot_config_set_path(plot_config_type * plot_config , const char * plot_path);
void               plot_config_set_plot_refcase(plot_config_type * plot_config , const char * plot_refcase);
void               plot_config_set_image_type(plot_config_type * plot_config , const char * plot_device);
void               plot_config_set_viewer(plot_config_type * plot_config , const char * plot_viewer);
void               plot_config_set_driver(plot_config_type * plot_config , const char * plot_driver);;
void               plot_config_set_errorbar_max(plot_config_type * plot_config , int errorbar_max);

int                plot_config_get_errorbar_max(const plot_config_type * plot_config );
bool               plot_config_get_plot_errorbar(const plot_config_type * plot_config );
int                plot_config_get_width(const plot_config_type * plot_config );
int                plot_config_get_height(const plot_config_type * plot_config );
const char  *      plot_config_get_path(const plot_config_type * plot_config );
const char  *      plot_config_get_plot_refcase(const plot_config_type * plot_config);
const char  *      plot_config_get_image_type(const plot_config_type * plot_config );
const char  *      plot_config_get_viewer(const plot_config_type * plot_config );
const char  *      plot_config_get_driver(const plot_config_type * plot_config );
void               plot_config_free( plot_config_type * plot_config);
plot_config_type * plot_config_alloc_default();
void               plot_config_init(plot_config_type * plot_config , const config_type * config );

void               plot_config_add_config_items( config_type * config );
void               plot_config_show_viewer_warning( plot_config_type * plot_config );

void               plot_config_toggle_logy( plot_config_type * plot_config );
void               plot_config_set_logy( plot_config_type * plot_config , bool logy);
bool               plot_config_get_logy( const plot_config_type * plot_config );


#endif
