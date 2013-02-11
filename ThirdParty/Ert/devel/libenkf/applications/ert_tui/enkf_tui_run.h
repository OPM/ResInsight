/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_tui_run.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef  __ENKF_TUI_RUN_H__
#define  __ENKF_TUI_RUN_H__

void enkf_tui_run_menu(void *);
void enkf_tui_run_exp(void *);
void enkf_tui_run_start(void *);
void enkf_tui_run_smoother(void *);
void enkf_tui_run_iterated_ES(void *);

#endif
