/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'qc_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef __QC_CONFIG_H__
#define __QC_CONFIG_H__

#ifdef __cplusplus 
extern "C" {
#endif

#include <ert/config/config.h>

  typedef struct qc_config_struct qc_config_type;
  
  qc_config_type * qc_config_alloc(const char * qc_path);
  void             qc_config_free();
  
  void             qc_config_set_path( qc_config_type * qc_config , const char * qc_path);
  const char *     qc_config_get_path( const qc_config_type * qc_config );
  void             qc_config_init( qc_config_type * qc_config , const config_type * config);


#ifdef __cplusplus 
}
#endif
#endif
