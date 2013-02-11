/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'active_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <util.h>
#include <enkf_types.h>
#include <enkf_config_node.h>
#include <ensemble_config.h>
#include <active_node.h>
#include <obs_vector.h>

/**
   This file implements the two most basic objects used in the mapping
   of active/inactive observations and variables. One of these nodes
   contains the information necessary to activate/deactivate one
   variable/observation.
*/


/**
   This struct implements the holding information for the
   activation/deactivation of one variable.
*/
  
struct active_var_struct {
  const enkf_config_node_type    * config_node;          /* The enkf_config_node instance this is all about - pointer to *shared* resource. */
  active_mode_type                 active_mode;         
  void                      	 * active_config;        /* An object (type depending on datatype of config_node) used to hold info abourt partly active variable. 
			    	 			    Owned by this object. If active_mode == all_active or active_mode == inactive, this can be NULL. */
  active_config_destructor_ftype * free_active_config;   /* Destructor for the active_config object, can be NULL if that object is NULL. */
};



/**
   Similar to active_var_struct, but for observations.
*/
struct active_obs_struct {
  const obs_vector_type       	  * obs_vector;             /* The obs_node instance this is all about - pointer to *shared* resource. */
  active_mode_type          	    active_mode;         
  void                      	  * active_config;        /* An object (type depending on datatype of obs_node) used to hold info abourt partly active variable. 
                   					     Owned by this object. If active_mode == all_active or active_mode == inactive, this can be NULL. */
  active_config_destructor_ftype  * free_active_config;   /* Destructor for the active_config object, can be NULL if that object is NULL. */
};




