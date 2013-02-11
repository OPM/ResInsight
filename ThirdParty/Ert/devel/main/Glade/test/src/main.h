/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'main.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GUI_MAIN_H__
#define __GUI_MAIN_H__


#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"

#include <enkf_main.h>
#include <model_config.h>
#include <ecl_config.h>
#include <path_fmt.h>
#include <util.h>
#include <ecl_grid.h>
#include <enkf_types.h>


#include <unistd.h>

enum
{
  TIMESTEP,
  ENSEMBLE_MEMBER,
  PROGRESS,
  PROGRESS_BUF,
  PROGRESS_ICON,
  COL_STATUS,
  ENKF_POINTER,
  N_COLS
};

enum
{
  SORT_TIMESTEP,
  SORT_ENSEMBLE_MEMBER
};

typedef enum _enkf_gui_run_node_type
{
  PARENT,
  CHILD
} enkf_gui_run_node;

typedef struct _enkf_gui_type
{
  enkf_gui_run_node type;
  gint ensemble_member;
  gint timestep;
} enkf_gui;


GtkTreeStore *enkf_gui_store_ensemble_member_parent ();
GtkTreeStore *enkf_gui_store_timestep_parent ();



#endif
