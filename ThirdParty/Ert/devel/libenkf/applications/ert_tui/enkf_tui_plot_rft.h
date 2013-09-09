/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_plot_rft.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_TUI_PLOT_RFT_H__
#define __ENKF_TUI_PLOT_RFT_H__

#ifdef __cplusplus 
extern "C" {
#endif

#include <ert/enkf/enkf_main.h>
  
  void   enkf_tui_plot_RFT_sim_all_MD( void * arg);
  void   enkf_tui_plot_RFT_sim_all_TVD( void * arg);
  void   enkf_tui_plot_select_RFT(const enkf_main_type * enkf_main , char ** _obs_key , int * _report_step);
  void   enkf_tui_plot_RFT_depth(void * arg);
  void   enkf_tui_plot_all_RFT( void * arg);
  
#ifdef __cplusplus 
}
#endif
#endif
