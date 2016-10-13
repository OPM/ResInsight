/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'local_config.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_LOCAL_CONFIG_H
#define ERT_LOCAL_CONFIG_H

#include <ert/util/stringlist.h>

#include <ert/ecl/ecl_grid.h>

#include <ert/analysis/analysis_module.h>

#include <ert/enkf/local_updatestep.h>
#include <ert/enkf/local_ministep.h>
#include <ert/enkf/ensemble_config.h>
#include <ert/enkf/enkf_obs.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct local_config_struct local_config_type;

local_config_type           * local_config_alloc( );
void                          local_config_clear( local_config_type * local_config );
void                          local_config_free( local_config_type * local_config );
local_ministep_type         * local_config_alloc_ministep( local_config_type * local_config , const char * key,  analysis_module_type* analysis_module );
local_ministep_type         * local_config_alloc_ministep_copy( local_config_type * local_config , const char * src_key , const char * new_key);
void                          local_config_set_default_updatestep( local_config_type * local_config , local_updatestep_type * update_step );
local_updatestep_type       * local_config_get_updatestep( const local_config_type * local_config );
local_ministep_type         * local_config_get_ministep( const local_config_type * local_config , const char * key);
void                          local_config_set_updatestep(local_config_type * local_config, int step1 , int step2 , const char * key);
void                          local_config_summary_fprintf( const local_config_type * local_config , const char * config_file);
local_obsdata_type          * local_config_alloc_obsdata( local_config_type * local_config , const char * obsdata_name );
bool                          local_config_has_obsdata( const local_config_type * local_config , const char * obsdata_name);
local_dataset_type          * local_config_alloc_dataset( local_config_type * local_config , const char * key );
bool                          local_config_has_dataset( const local_config_type * local_config , const char * key);
#ifdef __cplusplus
}
#endif
#endif
