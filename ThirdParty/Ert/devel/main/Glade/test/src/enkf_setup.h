/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_setup.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_SETUP_H__
#define __ENKF_SETUP_H__


#include <gtk/gtk.h>
#include <enkf_main.h>
#include <model_config.h>
#include <ecl_config.h>
#include <path_fmt.h>
#include <util.h>
#include <ecl_grid.h>
#include <enkf_types.h>
#include <sched_file.h>
#include <stringlist.h>
#include <stdlib.h>

enkf_main_type *enkf_setup_bootstrap(const char *enkf_config,
				     GtkWidget * win);


#endif
