/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ert_tui_jobs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/stringlist.h>

#include <ert/enkf/enkf_main.h>

#include <enkf_tui_plot.h>

void enkf_tui_plot_all_summary_JOB(void * self , const stringlist_type * args ) {
  enkf_main_type             * enkf_main       = enkf_main_safe_cast( self );
  int iens1 , iens2 , step1 , step2;   
  bool prediction_mode;
  iens1 = 0;
  iens2 = enkf_main_get_ensemble_size( enkf_main );
  step1 = 0;
  step2 = 0;
  prediction_mode = true;
  enkf_tui_plot_all_summary__( enkf_main , iens1 , iens2 , step1 , step2 , prediction_mode );
}

