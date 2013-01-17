/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_main_update.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <matrix.h>
#include <local_config.h>
#include <local_ministep.h>
#include <local_reportstep.h>
#include <enkf_node.h>
#include <enkf_state.h>
#include <enkf_analysis.h>
#include <analysis_config.h>

/**
   This file implements functions related to updating the state. The
   really low level functions related to the analysis is implemented
   in enkf_analysis.h.
   
   This file only implements code for updating, and not any data
   structures.
*/

#include "enkf_main_struct.h"


