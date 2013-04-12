/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'qc_module.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef __QC_MODULE_H__
#define __QC_MODULE_H__

#ifdef __cplusplus 
extern "C" {
#endif

#include <ert/config/config.h>

#include <ert/enkf/ert_workflow_list.h>
#include <ert/enkf/runpath_list.h>

  typedef struct qc_module_struct qc_module_type;

  bool                qc_module_has_workflow( const qc_module_type * qc_module );
  qc_module_type    * qc_module_alloc(ert_workflow_list_type * workflow_list ,  const char * qc_path);
  void                qc_module_free();
  bool                qc_module_run_workflow( qc_module_type * qc_module , void * self);
  runpath_list_type * qc_module_get_runpath_list( qc_module_type * qc_module );
  void                qc_module_set_path( qc_module_type * qc_module , const char * qc_path);
  const char    *     qc_module_get_path( const qc_module_type * qc_module );
  void                qc_module_init( qc_module_type * qc_module , const config_type * config);
  void                qc_module_export_runpath_list( const qc_module_type * qc_module );
  void                qc_module_add_config_items( config_type * config );
  void                qc_module_set_runpath_list_file( qc_module_type * qc_module , const char * filename);
  const char        * qc_module_get_runpath_list_file( qc_module_type * qc_module);



#ifdef __cplusplus 
}
#endif
#endif
