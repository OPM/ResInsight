/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'forward_model.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FORWARD_MODEL_H__
#define __FORWARD_MODEL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/subst_list.h>

#include <ert/job_queue/ext_joblist.h>

typedef struct  forward_model_struct forward_model_type ;


  stringlist_type        * forward_model_alloc_joblist( const forward_model_type * forward_model );
  const ext_joblist_type * forward_model_get_joblist(const forward_model_type * );
  void                     forward_model_clear( forward_model_type * forward_model );
  void                     forward_model_fprintf(const forward_model_type *  , FILE * );
  forward_model_type     * forward_model_alloc(const ext_joblist_type * ext_joblist);
  void                     forward_model_parse_init(forward_model_type * forward_model , const char * input_string );
  void                     forward_model_python_fprintf(const forward_model_type *  , const char * , const subst_list_type * );
  void                     forward_model_free( forward_model_type * );
  forward_model_type *     forward_model_alloc_copy(const forward_model_type * forward_model);
  void                     forward_model_iset_job_arg( forward_model_type * forward_model , int job_index , const char * arg , const char * value);
  ext_job_type           * forward_model_iget_job( forward_model_type * forward_model , int index);
  int                      forward_model_get_length( const forward_model_type * forward_model );


#ifdef __cplusplus
}
#endif
#endif
