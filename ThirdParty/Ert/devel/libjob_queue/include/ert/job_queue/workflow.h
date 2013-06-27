/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'workflow.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __WORKFLOW_H__
#define __WORKFLOW_H__


#ifdef __cplusplus
extern "C" {
#endif
  
#include <ert/util/subst_list.h>

#include <ert/config/config_error.h>

#include <ert/job_queue/workflow_joblist.h>

  typedef struct workflow_struct workflow_type;

  const config_error_type * workflow_get_last_error( const workflow_type * workflow);
  workflow_type           * workflow_alloc( const char * src_file , workflow_joblist_type * joblist);
  bool                      workflow_run(  workflow_type * workflow , void * self , bool verbose , const subst_list_type * context);
  void                      workflow_free( workflow_type * workflow );
  void                      workflow_free__( void * arg );

  int                       workflow_get_stack_size( const workflow_type * workflow );
  void                    * workflow_iget_stack_ptr( const workflow_type * workflow , int index);
  void                    * workflow_pop_stack( workflow_type * workflow );

#ifdef __cplusplus
}
#endif

#endif
