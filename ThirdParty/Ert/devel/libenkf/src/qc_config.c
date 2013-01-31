/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ert_qc_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>

#include <ert/util/util.h>

#include <ert/config/config.h>

#include <ert/enkf/config_keys.h>
#include <ert/enkf/qc_config.h>


struct qc_config_struct {
  char * qc_path;
};




qc_config_type * qc_config_alloc( const char * qc_path ) {
  qc_config_type * qc_config = util_malloc( sizeof * qc_config );
  qc_config->qc_path = NULL;
  qc_config_set_path( qc_config , qc_path );
  return qc_config;
}


void qc_config_free( qc_config_type * qc_config ) {
  util_safe_free( qc_config->qc_path );
  free( qc_config );
}


void qc_config_set_path( qc_config_type * qc_config , const char * qc_path) {
  qc_config->qc_path = util_realloc_string_copy( qc_config->qc_path , qc_path );
}


const char * qc_config_get_path( const qc_config_type * qc_config ) {
  return qc_config->qc_path;
}


void qc_config_init( qc_config_type * qc_config , const config_type * config) {
  if (config_item_set( config , QC_PATH_KEY )) 
    qc_config_set_path( qc_config , config_get_value( config , QC_PATH_KEY ));
}
