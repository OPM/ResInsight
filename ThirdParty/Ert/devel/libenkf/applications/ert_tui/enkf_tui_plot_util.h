/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_tui_plot_util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __ENKF_TUI_PLOT_UTIL_H__
#define __ENKF_TUI_PLOT_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/plot/plot.h>

#include <ert/enkf/plot_config.h>
  
  char      * enkf_tui_plot_alloc_plot_file(const plot_config_type * plot_config , const char * case_name , const char * base_name);
  void        enkf_tui_show_plot(plot_type * plot , const plot_config_type * plot_config , const char * file);
  plot_type * enkf_tui_plot_alloc(const plot_config_type * plot_config , const char * x_label , const char * y_label , const char * title , const char * file);


#ifdef __cplusplus
}
#endif
#endif
